TEMPLATE = subdirs
CONFIG += ordered

SUBDIRS += \
    src \
    # tests \
    examples/csv_demo \

# tests зависит от src
tests.depends = src
