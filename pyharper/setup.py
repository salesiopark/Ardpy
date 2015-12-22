from setuptools import setup, find_packages
setup(
    name = "pyharper",
    version = "1.0.0",
    packages = find_packages(),
    #scripts = ['say_hello.py'],

    # Project uses reStructuredText, so ensure that the docutils get
    # installed or upgraded on the target machine
    install_requires = ['smbus>=1.1'],
    # metadata for upload to PyPI
    author = "salesiopark",
    author_email = "salesiopark@gmail.com",
    description = "package for raspberry pi I2C communication with Arduino",
    license = "PSF",
    keywords = "arduino i2c raspberry pi",
    url = "http://github.com/Harper",   # project home page, if any

    # could also include long_description, download_url, classifiers, etc.
)
