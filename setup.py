# setup.py

import sys
import sysconfig
from setuptools import setup, Extension, find_packages
from setuptools.command.build_ext import build_ext
import setuptools
import os

try:
    import pybind11
except ImportError:
    raise ImportError("pybind11 is required to build this package. Install it via `pip install pybind11`.")

class get_pybind_include(object):
    """Helper class to determine the pybind11 include path.
    Delays importing pybind11 until it is actually installed,
    so that the `get_include()` method can be invoked.
    """

    def __init__(self, user=False):
        self.user = user

    def __str__(self):
        return pybind11.get_include(self.user)

ext_modules = [
    Extension(
        'nms.nms_module_optimized',  # Module name within the package
        ['nms/nms_module_optimized.cpp'],  # Path to the C++ source
        include_dirs=[
            # Path to pybind11 headers
            get_pybind_include(),
            get_pybind_include(user=True)
        ],
        language='c++',
        extra_compile_args=['-O3', '-std=c++11'],
    ),
]

# Determine if the platform is Windows
is_windows = sys.platform.startswith('win')

if is_windows:
    extra_compile_args = ['/O2', '/std:c++14']
    ext_modules[0].extra_compile_args = extra_compile_args
else:
    extra_compile_args = ['-O3', '-std=c++11']
    ext_modules[0].extra_compile_args = extra_compile_args

setup(
    name='nms_optimized',
    version='0.0.1',
    author='LMI AIS',
    author_email='your.email@example.com',
    description='A package for Non-Maximum Suppression implemented in C++ with Python bindings',
    long_description='',
    packages=find_packages(),
    ext_modules=ext_modules,
    install_requires=['pybind11>=2.5.0'],
    cmdclass={'build_ext': build_ext},
    zip_safe=False,
    classifiers=[
        'Programming Language :: Python :: 3',
        'Programming Language :: C++',
        'Operating System :: OS Independent',
    ],
    python_requires='>=3.6',
)