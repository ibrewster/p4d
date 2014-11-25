from distutils.command.build import build
from setuptools import setup, find_packages
from setuptools.command.install import install
from distutils.ccompiler import new_compiler
import os


def get_ext_modules():
    import py4d
    return [py4d.ffi.verifier.get_extension()]

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
        #make sure the library is built before getting the modules
        c = new_compiler()
        workdir = os.path.dirname(os.path.realpath(__file__)) + "/py4d/lib4d_sql"
        c.add_include_dir(workdir)

        sourceFiles = [workdir + "/" + x for x in os.listdir(workdir) if x.endswith(".c")]

        objects = c.compile(sourceFiles)
        c.link_shared_lib(objects, "4d_sql", output_dir="py4d/lib4d_sql")

        self.distribution.ext_modules = get_ext_modules()
        install.finalize_options(self)

setup(
    zip_safe=False,
    name="py4d",
    version="0.1",
    install_requires=["cffi", ],
    setup_requires=['cffi', ],
    packages=find_packages(),
    # need to include these files to be able to build our shared library
    package_data={'py4d': ['lib4d_sql/*.h', 'lib4d_sql/*.c', 'lib4d_sql/lib4d_sql.so'],},
    cmdclass={
        "build": CFFIBuild,
        "install": CFFIInstall,
    },
    author="Israel Brewster",
    author_email="israel@brewstersoft.com",
    url="http://www.brewstersoft.com"
)