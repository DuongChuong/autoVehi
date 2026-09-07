from setuptools import find_packages, setup

package_name = 'av_perception'

setup(
    name=package_name,
    version='0.0.0',
    packages=find_packages(exclude=['test']),
    data_files=[
        ('share/ament_index/resource_index/packages',
            ['resource/' + package_name]),
        ('share/' + package_name, ['package.xml']),
    ],
    install_requires=['setuptools'],
    zip_safe=True,
    maintainer='chuongday',
    maintainer_email='chuongday5624@gmail.com',
    description='TODO: Package description',
    license='BSD-3-Clause',
    extras_require={
        'test': [
            'pytest',
        ],
    },
    entry_points={
        'console_scripts': [
            # executable_name = module_name.script_name:function_name
            'yolo_detector = av_perception.yolo_node:main',
        ],
    },
)
