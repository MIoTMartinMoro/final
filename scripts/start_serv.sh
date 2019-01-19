#!/bin/bash

opkg install publish_mqtt_2.0.0-1_pistachio.ipk
echo "publish_mqtt instalado"
echo "esperando Servidor..."
python3 servidor_final.py