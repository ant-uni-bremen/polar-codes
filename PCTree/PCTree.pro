# Copyright 2017 Florian Lotze
#
# This file is part of PolarCodes
#
# SPDX-License-Identifier: GPL-3.0-or-later
#

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = PCTree
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    ArrayFuncs.cpp

HEADERS  += mainwindow.h \
    ArrayFuncs.h

FORMS    += mainwindow.ui
