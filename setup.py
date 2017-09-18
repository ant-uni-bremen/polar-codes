#!/usr/bin/env python

'''
Polar Code Cython kernels
~~~~~~~

This setup.py file exists to assist with Cython install.
It may be fragile in case assumptions and dependencies are not met.
It depends on various tools!
Python: setuptools, Cython, subprocess, shlex, inspect, os
Cython files must be located in ./gnuradio/fec/lib/pypolar!
C/C++ files must be located in '.'

This installs 1 additional Python module!
pypolar: Python Polar Code Wrapper
'''

from __future__ import print_function, division
from setuptools import setup
from setuptools.extension import Extension
from Cython.Build import cythonize
from Cython.Distutils import build_ext
import subprocess
import inspect
import os


def run_pkg_config(opts, params):
    return subprocess.check_output(("pkg-config", "--" + opts, params))


def sanitize_pkg_output(pkg_str):
    ps = pkg_str.split()
    ps = [i for i in ps if not i == '\n']
    return ps


def get_pkg_option(opt, name):
    ps = run_pkg_config(opt, name)
    return sanitize_pkg_output(ps)


def get_library_shared_objects(lib):
    lib_dirs = []
    lib_arg = get_pkg_option('libs', lib)
    for la in lib_arg:
        if '-L' in la:
            lib_dirs += [la.replace('-L', ''), ]
    return lib_dirs


def find_shared_objects(libraries):
    lib_dirs = []
    for l in libraries:
        lib_dirs += get_library_shared_objects(l)
    return lib_dirs


def get_library_headers(lib):
    shared_dirs = []
    comp_arg = get_pkg_option('cflags', lib)
    for c in comp_arg:
        shared_dirs += [c.replace('-I', ''), ]
    return shared_dirs


def find_headers(libraries):
    shared_dirs = []
    for l in libraries:
        shared_dirs += get_library_headers(l)
    return shared_dirs


def find_source_files(project_top_level_dir, targets, header_prefix='', impl_prefix=''):
    source_files = []
    for target in targets:
        header_file = os.path.join(project_top_level_dir, header_prefix, target + '.h')
        source_file = os.path.join(project_top_level_dir, impl_prefix, target + '.cpp')
        if not os.path.exists(header_file):
            raise ValueError('ERROR: Could not find header(' + header_file + ') file')
        if not os.path.exists(source_file):
            raise ValueError('ERROR: Could not find source(' + source_file + ') file')
        if not os.path.exists(header_file) or not os.path.exists(source_file):
            raise ValueError('ERROR: Could not find header(' + header_file + ') and source(' + source_file + ') files!')
        source_files += [source_file, ]
    return source_files


def get_project_top_level_dir():
    own_file_path = inspect.getfile(inspect.currentframe())
    own_file_path = os.path.abspath(own_file_path)  # make sure full path is available!
    path, filename = os.path.split(own_file_path)
    return path


def get_available_dependencies(deps):
    avail = []
    for d in deps:
        try:
            __import__(d)
            print(d)
            avail += [d, ]
        except ImportError:
            print('Python module:', d, 'not available!')
    return avail


project_top_level_dir = get_project_top_level_dir()
print('project TOP level directory:', project_top_level_dir)

# the actual classes to be compiled!
# assume those are the only additional libraries to link against.
libraries = ['PolarCode', ]


source_files = [os.path.join(project_top_level_dir, 'pypolar/pypolar.pyx'), ]
print('source_files:\n', '\n'.join(source_files))

include_dirs = [os.path.join(project_top_level_dir, 'pypolar')]
include_dirs += find_headers(libraries)
library_dirs = find_shared_objects(libraries)

print('include_dirs:\n', '\n'.join(include_dirs))
print('library_dirs:\n', '\n'.join(library_dirs))


ext_modules = [
    Extension("pypolar",
              include_dirs=include_dirs,
              library_dirs=library_dirs,
              sources=source_files,
              libraries=libraries,
              language="c++",
              extra_compile_args=["-std=c++11", "-Ofast", ],
              #extra_link_args=["-std=c++11", ],
              )
]


# setuptools Doc: https://pythonhosted.org/an_example_pypi_project/setuptools.html
setup(
    name="python-polar-code",
    version="0.0.1",
    author="Johannes Demel",
    author_email="demel@ant.uni-bremen.de",
    description='Python bindings for Polar Code',
    keywords="Polar Code",
    packages=['pypolar'],
    install_requires=['numpy', ],
    cmdclass={'build_ext': build_ext},
    ext_modules=cythonize(ext_modules),
    # ext_modules=cythonize(ext_modules, annotate=True)  # this produces an .HTML which helps identify function overhead.
)

