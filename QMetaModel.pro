TEMPLATE = subdirs
CONFIG += ordered

SUBDIRS += \
    src \
    tests \

# tests зависит от src
tests.depends = src
