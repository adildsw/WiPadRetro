from setuptools import setup, find_packages
import os

with open('requirements.txt') as f:
    requirements = f.read().splitlines()

version = '0.1.0'

setup(
    name='WiPadRetroLink',
    version=version,
    author='Adil Rahman',
    author_email='adildsw@gmail.com',
    description='Receiver and input mapper for WiPadRetro',
    url='http://github.com/adildsw/WiPadRetro',
    packages=find_packages(),
    install_requires=requirements,
    entry_points={
        'console_scripts': [
            'wprlink=WiPadRetroLink.app:main',
        ],
    },
    python_requires='>=3.6',
    include_package_data=True,
    keywords='your, project, keywords',
)

