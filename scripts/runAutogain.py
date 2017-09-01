#!/usr/bin/env python

from AdOpt import autogain
a = autogain.autogain(app)
a.doit( app.ccd39.xbin())

