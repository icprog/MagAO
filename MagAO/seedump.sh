#!/bin/sh


#This queries for DIMM seeing
#commands provided by Gabriel Prieto

echo "use weather; select tm as datetime, se as fwhm, el as elevation from dimm_data where un = 0 order by tm desc limit 1;" |  mysql --connect-timeout=1 -h 139.229.105.25 -u read_data_ao > /tmp/dimmdump

echo "use weather; select tm as datetime, fw as fw, el as elevation from magellan_data where un = 0 order by tm desc limit 1;" |  mysql --connect-timeout=1 -h 139.229.105.25 -u read_data_ao > /tmp/mag1dump

echo "use weather; select tm as datetime, fw as fw, el as elevation from magellan_data where un = 1 order by tm desc limit 1;" |  mysql --connect-timeout=1 -h 139.229.105.25 -u read_data_ao > /tmp/mag2dump
