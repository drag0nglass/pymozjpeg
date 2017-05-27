#include <Python.h>
#include <stdio.h>

#include "cdjpeg.h"
#include "jversion.h"
#include "jconfigint.h"


static const char * const cdjpeg_message_table[] = {
#include "cderror.h"
  NULL
};

static PyObject* get_jpeg_quality(PyObject *self, PyObject *args)
{
    const unsigned char *input_data;
    unsigned long input_data_size;
    struct jpeg_decompress_struct dinfo;
    struct jpeg_error_mgr jerr;
    int quality = 0;

    if (!PyArg_ParseTuple(args, "s#", &input_data, &input_data_size))
      return NULL;

    dinfo.err = jpeg_std_error(&jerr);

    jpeg_create_decompress(&dinfo);
    jpeg_mem_src(&dinfo, input_data, input_data_size);
    jpeg_read_header(&dinfo, TRUE);
    jpeg_start_decompress(&dinfo);

    for (unsigned int i=0; i < NUM_QUANT_TBLS; i++)
    {
      if (dinfo.quant_tbl_ptrs[i] != NULL)
        for (unsigned int j=0; j < DCTSIZE2; j++)
          quality+=dinfo.quant_tbl_ptrs[i]->quantval[j];
    }

    return Py_BuildValue("i", quality);
}

static PyObject* cjpeg(PyObject *self, PyObject *args)
{
  const unsigned char *input_data;
  unsigned long input_data_size;

  struct jpeg_compress_struct cinfo;
  struct jpeg_decompress_struct dinfo;
  struct jpeg_error_mgr jerr;
  unsigned char *outbuffer = NULL;
  unsigned long outsize = 0;
  JSAMPARRAY buffer;
  int row_stride;
  int quality = 75;

  if (!PyArg_ParseTuple(args, "s#|i", &input_data, &input_data_size, &quality))
    return NULL;

  dinfo.err = cinfo.err = jpeg_std_error(&jerr);
  jerr.addon_message_table = cdjpeg_message_table; 
  jerr.first_addon_message = JMSG_FIRSTADDONCODE;
  jerr.last_addon_message = JMSG_LASTADDONCODE;

  jpeg_create_decompress(&dinfo);
  jpeg_mem_src(&dinfo, input_data, input_data_size);
  jpeg_read_header(&dinfo, TRUE);
  jpeg_start_decompress(&dinfo);

  jpeg_create_compress(&cinfo);
  jpeg_mem_dest(&cinfo, &outbuffer, &outsize);

  cinfo.image_width = dinfo.image_width;
  cinfo.image_height = dinfo.image_height;
  cinfo.input_components = 3;
  cinfo.in_color_space = JCS_RGB;

  jpeg_set_quality(&cinfo, quality, TRUE);

  jpeg_c_set_int_param(&cinfo, JINT_COMPRESS_PROFILE, JCP_FASTEST);
  jpeg_set_defaults(&cinfo);

  jpeg_start_compress(&cinfo, TRUE);

  row_stride = cinfo.image_width * cinfo.input_components;
  buffer = (*dinfo.mem->alloc_sarray)((j_common_ptr) &dinfo, JPOOL_IMAGE, row_stride, 1);
  while(cinfo.next_scanline < cinfo.image_height) {
    
    jpeg_read_scanlines(&dinfo, buffer, 1);
    jpeg_write_scanlines(&cinfo, buffer, 1);
  }

  jpeg_finish_decompress(&dinfo);
  jpeg_destroy_decompress(&dinfo);

  jpeg_finish_compress(&cinfo);
  jpeg_destroy_compress(&cinfo);

  return Py_BuildValue("s#", outbuffer, outsize);
}

//-----------------------------------------------------------------------------
static PyMethodDef pymozjpeg_methods[] = {
  {
    "cjpeg",
    cjpeg,
    METH_VARARGS,
    "simple cjpeg-like interface for the JPEG compressor"
  },
  {
    "get_jpeg_quality",
    get_jpeg_quality,
    METH_VARARGS,
    "determine JPEG quality"
  },
  {NULL, NULL, 0, NULL}
};

//-----------------------------------------------------------------------------
#if PY_MAJOR_VERSION < 3

PyMODINIT_FUNC init_pymozjpeg(void)
{
  (void) Py_InitModule("_pymozjpeg", pymozjpeg_methods);
}

#else /* PY_MAJOR_VERSION >= 3 */

static struct PyModuleDef pymozjpeg_module_def = {
  PyModuleDef_HEAD_INIT,
  "_pymozjpeg",
  "\"_pymozjpeg\" module",
  -1,
  pymozjpeg_methods
};

PyMODINIT_FUNC PyInit__pymozjpeg(void)
{
  return PyModule_Create(&pymozjpeg_module_def);
}

#endif /* PY_MAJOR_VERSION >= 3 */