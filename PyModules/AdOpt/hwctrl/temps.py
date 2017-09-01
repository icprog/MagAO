#!/usr/bin/env python2
#
#@File: temperature.py

from AdOpt import cfg
import numpy, time

class temps:

    def __init__(self, app):

        self.vars = {}
        self.items = []
        self.app = app

        class item:
            pass

        self.powerboard_t_num = cfg.cfg['powerboard']['t_num'].Value()
        self.flowerpot_t_num  = cfg.cfg['picctrl']['temps.num_items'].Value()
        self.flowerpot_h_num  = cfg.cfg['picctrl']['humidity.num_items'].Value()
        self.flowerpot_d_num  = cfg.cfg['picctrl']['dews.num_items'].Value()

        # Elaborate parsing of cfg file follows... any way to simplify?
        powerboard_dict  = {'task': 'powerboard', 'num':self.powerboard_t_num, 'var':'TEMPS',\
                            'nameTemplate':'temp%d_name', 'alarmTemplate':'temp%d_alarm', 'unitTemplate':'temp%d_unit'}
        flowerpot_t_dict = {'task': 'picctrl', 'num':self.flowerpot_t_num, 'var':'TEMPS', \
                            'nameTemplate':'temps.item%d_name', 'alarmTemplate':'temps.item%d_alarm', 'unitTemplate':'temps.item%d_unit'}
        flowerpot_h_dict = {'task': 'picctrl', 'num':self.flowerpot_h_num, 'var':'HUM', \
                            'nameTemplate':'humidity.item%d_name', 'alarmTemplate':'humidity.item%d_alarm', 'unitTemplate':'humidity.item%d_unit'}
        flowerpot_d_dict = {'task': 'picctrl', 'num':self.flowerpot_d_num, 'var':'DEW', \
                            'nameTemplate':'dews.item%d_name', 'alarmTemplate':'dews.item%d_alarm', 'unitTemplate':'dews.item%d_unit'}

        for d in [powerboard_dict, flowerpot_t_dict, flowerpot_h_dict, flowerpot_d_dict]:
            c = cfg.cfg[d['task']]
            varname = cfg.varname(d['task'], d['var'])
            for i in range(d['num']):
                t = item()
                t.name = c[d['nameTemplate'] % (i+1)].Value()
                t.varname = varname
                t.index = i
                t.unit  = c[d['unitTemplate'] % (i+1)].Value()
                self.items.append(t)

            self.vars[varname] =  app.ReadVar(varname)
            app.VarNotif( self.vars[varname])

        # Now the internal CCD temperatures
        varname = cfg.varname('ccd39', "TEMPS")
        t = item()
        t.name = 'LittleJoe 39 - internal'
        t.varname = varname
        t.index = 0
        t.unit = 'C'
        self.items.append(t)
        self.vars[varname] = self.app.ReadVar(varname)
        self.app.VarNotif(self.vars[varname])

        varname = cfg.varname('ccd47', "TEMPS")
        t = item()
        t.name = 'LittleJoe 47 - internal'
        t.varname = varname
        t.index =0
        t.unit = 'C'
        self.vars[varname] = self.app.ReadVar(varname)
        self.app.VarNotif(self.vars[varname])

    def temps(self):
         '''
         Returns all the current temperatures as an associative array.
         '''

         temps = {}
         for item in self.items:
             v = self.vars[item.varname].Value()
             try:
                 t = v[item.index]
             except:
                 t = v
             temps[item.name] = t
         return temps

    def units(self):
         '''
         Returns all the units used for the current temperatures, as an associative array
         '''

         units = {}
         for item in self.items:
             units[item.name] = item.unit
         return units
              





              





