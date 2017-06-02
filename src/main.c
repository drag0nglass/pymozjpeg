#include <Python.h>
#include <stdio.h>

#include "cdjpeg.h"
#include "jversion.h"
#include "jconfigint.h"
#include <setjmp.h>

typedef struct _error_mgr {
  struct jpeg_error_mgr pub;
  jmp_buf jb;
} error_mgr;

static void error_emit_exception(j_common_ptr cinfo)
{
  error_mgr *myerr = (error_mgr *)cinfo->err;
  longjmp(myerr->jb, 1);
}

void my_emit_message (j_common_ptr cinfo, int msg_level)
{

}

GLOBAL(struct jpeg_error_mgr *)
pymozjpeg_error(error_mgr *err) 
{
  jpeg_std_error((struct jpeg_error_mgr *)err);
  err->pub.trace_level = 0;
  err->pub.error_exit = error_emit_exception;

  return (struct jpeg_error_mgr *)err;
}
 
/* Code from ImageMagik library */
static PyObject* get_jpeg_quality(PyObject *self, PyObject *args)
{
    const unsigned char *input_data;
    unsigned long input_data_size;
    struct jpeg_decompress_struct dinfo;
    error_mgr jerr;
    int quality = 0;

    ssize_t j, qvalue, sum;
    register ssize_t i;

    if (!PyArg_ParseTuple(args, "s#", &input_data, &input_data_size))
      return NULL;

    dinfo.err = pymozjpeg_error(&jerr);
    dinfo.global_state = 0;

    if(setjmp(jerr.jb)) {
      PyErr_SetString(PyExc_ValueError, jerr.pub.jpeg_message_table[jerr.pub.last_jpeg_message]);
      if (dinfo.global_state != 0)
        jpeg_destroy_decompress(&dinfo);
      return NULL;
    }

    jpeg_create_decompress(&dinfo);
    jpeg_mem_src(&dinfo, input_data, input_data_size);
    jpeg_read_header(&dinfo, TRUE);
    jpeg_start_decompress(&dinfo);

    sum=0;
    for (i=0; i < NUM_QUANT_TBLS; i++)
    {
      if (dinfo.quant_tbl_ptrs[i] != NULL)
        for (j=0; j < DCTSIZE2; j++)
          sum+=dinfo.quant_tbl_ptrs[i]->quantval[j];
    }
    if ((dinfo.quant_tbl_ptrs[0] != NULL) &&
        (dinfo.quant_tbl_ptrs[1] != NULL))
      {
        ssize_t
          hash[101] =
          {
            1020, 1015,  932,  848,  780,  735,  702,  679,  660,  645,
             632,  623,  613,  607,  600,  594,  589,  585,  581,  571,
             555,  542,  529,  514,  494,  474,  457,  439,  424,  410,
             397,  386,  373,  364,  351,  341,  334,  324,  317,  309,
             299,  294,  287,  279,  274,  267,  262,  257,  251,  247,
             243,  237,  232,  227,  222,  217,  213,  207,  202,  198,
             192,  188,  183,  177,  173,  168,  163,  157,  153,  148,
             143,  139,  132,  128,  125,  119,  115,  108,  104,   99,
              94,   90,   84,   79,   74,   70,   64,   59,   55,   49,
              45,   40,   34,   30,   25,   20,   15,   11,    6,    4,
               0
          },
          sums[101] =
          {
            32640, 32635, 32266, 31495, 30665, 29804, 29146, 28599, 28104,
            27670, 27225, 26725, 26210, 25716, 25240, 24789, 24373, 23946,
            23572, 22846, 21801, 20842, 19949, 19121, 18386, 17651, 16998,
            16349, 15800, 15247, 14783, 14321, 13859, 13535, 13081, 12702,
            12423, 12056, 11779, 11513, 11135, 10955, 10676, 10392, 10208,
             9928,  9747,  9564,  9369,  9193,  9017,  8822,  8639,  8458,
             8270,  8084,  7896,  7710,  7527,  7347,  7156,  6977,  6788,
             6607,  6422,  6236,  6054,  5867,  5684,  5495,  5305,  5128,
             4945,  4751,  4638,  4442,  4248,  4065,  3888,  3698,  3509,
             3326,  3139,  2957,  2775,  2586,  2405,  2216,  2037,  1846,
             1666,  1483,  1297,  1109,   927,   735,   554,   375,   201,
              128,     0
          };

        qvalue=(ssize_t) (dinfo.quant_tbl_ptrs[0]->quantval[2]+
          dinfo.quant_tbl_ptrs[0]->quantval[53]+
          dinfo.quant_tbl_ptrs[1]->quantval[0]+
          dinfo.quant_tbl_ptrs[1]->quantval[DCTSIZE2-1]);
        for (i=0; i < 100; i++)
        {
          if ((qvalue < hash[i]) && (sum < sums[i]))
            continue;
          if (((qvalue <= hash[i]) && (sum <= sums[i])) || (i >= 50))
            quality=(size_t) i+1;
          break;
        }
      }
    else
      if (dinfo.quant_tbl_ptrs[0] != NULL)
        {
          ssize_t
            hash[101] =
            {
              510,  505,  422,  380,  355,  338,  326,  318,  311,  305,
              300,  297,  293,  291,  288,  286,  284,  283,  281,  280,
              279,  278,  277,  273,  262,  251,  243,  233,  225,  218,
              211,  205,  198,  193,  186,  181,  177,  172,  168,  164,
              158,  156,  152,  148,  145,  142,  139,  136,  133,  131,
              129,  126,  123,  120,  118,  115,  113,  110,  107,  105,
              102,  100,   97,   94,   92,   89,   87,   83,   81,   79,
               76,   74,   70,   68,   66,   63,   61,   57,   55,   52,
               50,   48,   44,   42,   39,   37,   34,   31,   29,   26,
               24,   21,   18,   16,   13,   11,    8,    6,    3,    2,
                0
            },
            sums[101] =
            {
              16320, 16315, 15946, 15277, 14655, 14073, 13623, 13230, 12859,
              12560, 12240, 11861, 11456, 11081, 10714, 10360, 10027,  9679,
               9368,  9056,  8680,  8331,  7995,  7668,  7376,  7084,  6823,
               6562,  6345,  6125,  5939,  5756,  5571,  5421,  5240,  5086,
               4976,  4829,  4719,  4616,  4463,  4393,  4280,  4166,  4092,
               3980,  3909,  3835,  3755,  3688,  3621,  3541,  3467,  3396,
               3323,  3247,  3170,  3096,  3021,  2952,  2874,  2804,  2727,
               2657,  2583,  2509,  2437,  2362,  2290,  2211,  2136,  2068,
               1996,  1915,  1858,  1773,  1692,  1620,  1552,  1477,  1398,
               1326,  1251,  1179,  1109,  1031,   961,   884,   814,   736,
                667,   592,   518,   441,   369,   292,   221,   151,    86,
                 64,     0
            };

          qvalue=(ssize_t) (dinfo.quant_tbl_ptrs[0]->quantval[2]+
            dinfo.quant_tbl_ptrs[0]->quantval[53]);
          for (i=0; i < 100; i++)
          {
            if ((qvalue < hash[i]) && (sum < sums[i]))
              continue;
            if (((qvalue <= hash[i]) && (sum <= sums[i])) || (i >= 50))
              quality=(size_t)i+1;
            break;
          }
        }

    jpeg_destroy_decompress(&dinfo);

    return Py_BuildValue("i", quality);
}

static PyObject* cjpeg(PyObject *self, PyObject *args)
{
  const unsigned char *input_data;
  unsigned long input_data_size;

  struct jpeg_compress_struct cinfo;
  struct jpeg_decompress_struct dinfo;
  error_mgr jerr;
  unsigned char *outbuffer = NULL;
  unsigned long outsize = 0;
  JSAMPARRAY buffer;
  int row_stride;
  int quality = 75;
  int fast_encoding = 1;

  if (!PyArg_ParseTuple(args, "s#|ii", &input_data, &input_data_size, &quality, &fast_encoding))
    return NULL;

  if (input_data_size < 134) {
    PyErr_SetString(PyExc_ValueError, "Not enough data");
    return NULL;
  }

  dinfo.global_state = cinfo.global_state = 0;
  dinfo.err = cinfo.err = pymozjpeg_error(&jerr);

  if(setjmp(jerr.jb)) {
    PyErr_SetString(PyExc_ValueError, jerr.pub.jpeg_message_table[jerr.pub.last_jpeg_message]);
    if (dinfo.global_state != 0)
      jpeg_destroy_decompress(&dinfo);
    if (cinfo.global_state != 0)
      jpeg_destroy_compress(&cinfo);
    return NULL;
  }

  jpeg_create_decompress(&dinfo);
  jpeg_mem_src(&dinfo, input_data, input_data_size);

  jpeg_save_markers(&dinfo, JPEG_COM, 0xFFFF);
  
  for (int m = 0; m < 16; m++)
    jpeg_save_markers(&dinfo, JPEG_APP0 + m, 0xFFFF);

  jpeg_read_header(&dinfo, TRUE);

  dinfo.raw_data_out = FALSE;
  jpeg_start_decompress(&dinfo);

  jpeg_create_compress(&cinfo);

  cinfo.in_color_space = dinfo.out_color_space;
  cinfo.input_components = dinfo.output_components;
  cinfo.data_precision = dinfo.data_precision;
  cinfo.image_width = dinfo.image_width;
  cinfo.image_height = dinfo.image_height;

  cinfo.raw_data_in = FALSE;

  jpeg_set_defaults(&cinfo);

  if (fast_encoding) {
      jpeg_c_set_int_param(&cinfo, JINT_COMPRESS_PROFILE, JCP_FASTEST);
      jpeg_set_defaults(&cinfo);
  } else {
      jpeg_c_set_int_param(&cinfo, JINT_COMPRESS_PROFILE, JCP_MAX_COMPRESSION);
  }

  jpeg_set_quality(&cinfo, quality, TRUE);
  
  jpeg_mem_dest(&cinfo, &outbuffer, &outsize);
  jpeg_start_compress(&cinfo, TRUE);

  row_stride = dinfo.image_width * dinfo.output_components;
  buffer = (*dinfo.mem->alloc_sarray)((j_common_ptr) &dinfo, JPOOL_IMAGE, row_stride, 1);
  while(dinfo.output_scanline < dinfo.output_height) {
    JDIMENSION num_scanlines = jpeg_read_scanlines(&dinfo, buffer, 1);
    jpeg_write_scanlines(&cinfo, buffer, num_scanlines);
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