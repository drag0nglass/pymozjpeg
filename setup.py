#!/usr/bin/env python
import os
import sys
import urllib2
import tarfile
import subprocess
import shutil
import re
from setuptools import setup, Extension, Command
from distutils.command import build_py, build_ext, clean
from distutils.util import get_platform

WITH_SIMD = True
WITH_ARITH_ENC = True
WITH_ARITH_DEC = True

MOZJPEG_URL = 'https://github.com/mozilla/mozjpeg/releases/download/v3.2/mozjpeg-3.2-release-source.tar.gz'
JPEG_SOURCES = ['jcapimin.c', 'jcapistd.c', 'jccoefct.c', 'jccolor.c', 'jcdctmgr.c', 'jcext.c',
                'jchuff.c', 'jcinit.c', 'jcmainct.c', 'jcmarker.c', 'jcmaster.c', 'jcomapi.c',
                'jcparam.c', 'jcphuff.c', 'jcprepct.c', 'jcsample.c', 'jctrans.c', 'jdapimin.c',
                'jdapistd.c', 'jdatadst.c', 'jdatasrc.c', 'jdcoefct.c', 'jdcolor.c', 'jddctmgr.c',
                'jdhuff.c', 'jdinput.c', 'jdmainct.c', 'jdmarker.c', 'jdmaster.c', 'jdmerge.c',
                'jdphuff.c', 'jdpostct.c', 'jdsample.c', 'jdtrans.c', 'jerror.c', 'jfdctflt.c',
                'jfdctfst.c', 'jfdctint.c', 'jidctflt.c', 'jidctfst.c', 'jidctint.c', 'jidctred.c',
                'jquant1.c', 'jquant2.c', 'jutils.c', 'jmemmgr.c', 'jmemnobs.c']

if WITH_ARITH_ENC or WITH_ARITH_DEC:
    JPEG_SOURCES.append('jaricom.c')

if WITH_ARITH_ENC:
    JPEG_SOURCES.append('jcarith.c')

if WITH_ARITH_DEC:
    JPEG_SOURCES.append('jdarith.c')

NAFLAGS = ['-I', os.path.abspath('./mozjpeg/simd/')+'/', '-I', os.path.abspath('./mozjpeg/win/')+'/']
JPEG_SIMD_SOURCES = []
if WITH_SIMD:
  if '64' in get_platform():
      JPEG_SIMD_SOURCES = ['jfdctflt-sse-64.asm', 'jccolor-sse2-64.asm', 'jcgray-sse2-64.asm',
                           'jchuff-sse2-64.asm', 'jcsample-sse2-64.asm', 'jdcolor-sse2-64.asm',
                           'jdmerge-sse2-64.asm', 'jdsample-sse2-64.asm', 'jfdctfst-sse2-64.asm',
                           'jfdctint-sse2-64.asm', 'jidctflt-sse2-64.asm', 'jidctfst-sse2-64.asm',
                           'jidctint-sse2-64.asm', 'jidctred-sse2-64.asm', 'jquantf-sse2-64.asm',
                           'jquanti-sse2-64.asm', 'jsimd_x86_64.c']
  else:
      JPEG_SIMD_SOURCES = ['jsimdcpu.asm', 'jfdctflt-3dn.asm', 'jidctflt-3dn.asm', 'jquant-3dn.asm',
                           'jccolor-mmx.asm', 'jcgray-mmx.asm', 'jcsample-mmx.asm', 'jdcolor-mmx.asm',
                           'jdmerge-mmx.asm', 'jdsample-mmx.asm', 'jfdctfst-mmx.asm',
                           'jfdctint-mmx.asm', 'jidctfst-mmx.asm', 'jidctint-mmx.asm',
                           'jidctred-mmx.asm', 'jquant-mmx.asm', 'jfdctflt-sse.asm',
                           'jidctflt-sse.asm', 'jquant-sse.asm', 'jccolor-sse2.asm',
                           'jcgray-sse2.asm', 'jchuff-sse2.asm', 'jcsample-sse2.asm',
                           'jdcolor-sse2.asm', 'jdmerge-sse2.asm', 'jdsample-sse2.asm',
                           'jfdctfst-sse2.asm', 'jfdctint-sse2.asm', 'jidctflt-sse2.asm',
                           'jidctfst-sse2.asm', 'jidctint-sse2.asm', 'jidctred-sse2.asm',
                           'jquantf-sse2.asm', 'jquanti-sse2.asm', 'jsimd_i386.c']

sources = ['src/main.c', 'mozjpeg/cdjpeg.c', 'mozjpeg/rdjpeg.c', 'mozjpeg/rdpng.c',
           'mozjpeg/rdbmp.c','mozjpeg/rdgif.c', 'mozjpeg/rdppm.c', 'mozjpeg/rdrle.c']
extra_objects = []
all_sources = sources + \
              ['mozjpeg/'+s for s in JPEG_SOURCES] + \
              ['mozjpeg/simd/'+s for s in JPEG_SIMD_SOURCES]

pymozjpeg_module = Extension('pymozjpeg/_pymozjpeg',
                             sources=all_sources,
                             extra_objects=extra_objects,
                             depends=extra_objects,
                             include_dirs=[
                                os.path.join(os.path.dirname(os.path.abspath(__file__)), 'src'),
                                os.path.join(os.path.dirname(os.path.abspath(__file__)), 'mozjpeg'),
                                ])


class my_build_ext(build_ext.build_ext):
    
    def build_extensions(self):
        global NAFLAGS
        MOZJPEG_ARCHIVE = "mozjpeg.tar.gz"
        if not os.path.exists(MOZJPEG_ARCHIVE):
            print "Downloading mozjpeg..."
            with open("mozjpeg.tar.gz", "wb+") as out:
                response = urllib2.urlopen(MOZJPEG_URL)
                out.write(response.read())
        if not os.path.exists("./mozjpeg/"):
            print "Extracting mozjpeg..."
            tar = tarfile.open(MOZJPEG_ARCHIVE, "r:gz")
            tar.extractall()
            tar.close()
        if not os.path.exists("./mozjpeg/jconfigint.h"):
            subprocess.Popen(['sh', 'configure'], cwd=os.path.abspath('./mozjpeg/')).wait()
        for ext in self.extensions:
            if ext.name == 'pymozjpeg/_pymozjpeg':
                makefile = open('./mozjpeg/simd/Makefile', 'r').read()

                NAFLAGS += re.findall(r'NAFLAGS =(.+)', makefile)[0].strip().split(' ')
                NASM = re.findall(r'NASM =(.+)', makefile)[0].strip()
                for src in reversed(ext.sources):
                  if '.asm' in src:
                    del ext.sources[ext.sources.index(src)]
                    cmd = [NASM]+NAFLAGS+['-o', src.replace('.asm', '.o'), src]
                    subprocess.Popen(cmd).wait()
                    ext.extra_objects.append(src.replace('.asm', '.o'))
        build_ext.build_ext.build_extensions(self)


class my_clean(clean.clean):
    def run(self):
        clean.clean.run(self)
        shutil.rmtree('./build/')
        shutil.rmtree('./mozjpeg/')
        os.remove("mozjpeg.tar.gz")


if __name__ == '__main__':
    setup(name='pymozjpeg',
          version='0.1.0',
          description='mozjpeg Python bindings',
          long_description='Really, the funniest around.',
          classifiers=[
            'Development Status :: 4 - Beta',
            'License :: OSI Approved :: MIT License',
            'Programming Language :: Python :: 2.7',
            'Topic :: System :: Archiving :: Compression',
            'Topic :: Software Development :: Libraries :: Python Modules',
          ],
          keywords='jpeg mozjpeg image compression binding',
          url='https://github.com/sppps/pymozjpeg',
          author='Sergey S. Gogin',
          author_email='sppps@sppps.ru',
          license='MIT',
          packages=['pymozjpeg'],
          ext_modules=[pymozjpeg_module],
          cmdclass={'build_ext': my_build_ext, 'clean': my_clean},
          include_package_data=True,
          zip_safe=False)
