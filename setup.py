from distutils.command.build import build
from setuptools import setup, find_packages
from setuptools.command.install import install
from distutils.ccompiler import new_compiler
import os


def get_ext_modules():
    import p4d
    return [p4d.ffi.verifier.get_extension()]

class CFFIBuild(build):
    #----------------------------------------------------------------------
    def finalize_options(self):
        """"""
        self.distribution.ext_modules = get_ext_modules()
        build.finalize_options(self)

class CFFIInstall(install):
    #----------------------------------------------------------------------
    def finalize_options(self):
        """"""
        self.distribution.ext_modules = get_ext_modules()
        install.finalize_options(self)

setup(
    zip_safe=False,
    name="p4d",
    version="0.7",
    install_requires=["cffi", ],
    setup_requires=['cffi', ],
    packages=find_packages(),
    # need to include these files to be able to build our shared library
    package_data={'p4d': ['py_fourd.h'],},
    cmdclass={
        "build": CFFIBuild,
        "install": CFFIInstall,
    },
    author="Israel Brewster",
    author_email="israel@brewstersoft.com",
    url="https://github.com/ibrewster/p4d",

    description="Python DBI module for the 4D database",
    long_description="This module provides a Python Database API v2.0 compliant driver for the 4D (4th Dimension, http://www.4d.com ) database. Based off of C library code provided by 4th Dimension and implemented using CFFI",

    license='BSD',
    classifiers=['Development Status :: 4 - Beta',
                 'License :: OSI Approved :: BSD License',
                 'Intended Audience :: Developers',
                 'Topic :: Database'],
    keywords='datababase drivers DBI 4d'

)
