from setuptools import Extension, setup

module = Extension("symnmf_c_api", sources=['symnmf.c', 'symnmfmodule.c'])
setup(
    name='symnmf_c_api',
     version='1.0',
     description='Python wrapper for SymNMF C extension',
     ext_modules=[module])