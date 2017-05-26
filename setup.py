#!/usr/bin/env python
import os
import sys
import urllib2
import tarfile
import subprocess
import shutil
from setuptools import setup, Extension, Command
from distutils.command import build_py, build_ext, clean
from distutils.dep_util import newer

MOZJPEG_URL = 'https://github.com/mozilla/mozjpeg/releases/download/v3.2/mozjpeg-3.2-release-source.tar.gz'

sources = ['src/main.c', 'mozjpeg/cdjpeg.c', 'mozjpeg/rdjpeg.c', 'mozjpeg/rdpng.c',
           'mozjpeg/rdbmp.c','mozjpeg/rdgif.c', 'mozjpeg/rdppm.c', 'mozjpeg/rdrle.c']
extra_objects = ['mozjpeg/.libs/libjpeg.a', 'mozjpeg/.libs/libturbojpeg.a']

pymozjpeg_module = Extension('pymozjpeg/_pymozjpeg',
                             sources=sources,
                             extra_objects=extra_objects,
                             depends=extra_objects,
                             include_dirs=[
                                os.path.join(os.path.dirname(os.path.abspath(__file__)), 'src'),
                                os.path.join(os.path.dirname(os.path.abspath(__file__)), 'mozjpeg'),
                                ])


class my_build_ext(build_ext.build_ext):
    def build_extensions(self):
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
        if not os.path.exists('mozjpeg/.libs/libjpeg.a'):
            subprocess.Popen(['bash', 'configure'], cwd=os.path.abspath('mozjpeg/')).wait()
            subprocess.Popen(['make'], cwd=os.path.abspath('mozjpeg/')).wait()
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
