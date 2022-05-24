from datetime import time
import random

from django.contrib.auth.hashers import make_password



print('Generating dummy data')

'''CREACIÓN DE ADMINISTRACION'''
print('Generating Admin Account')
from django.contrib.auth.models import User
User.objects.create_superuser(username='admin', email='admin@contapy.com', password="secret", is_active=True)


'''CREACIÓN DE AULAS'''
from classroom.models import Classroom
print('Generating Classroom Entities')
atc1=Classroom(code='LVP7001', name="ATC 1")
atc2=Classroom(code='LVP7002', name="ATC 2")
atc3=Classroom(code='LVP7003', name="ATC 3")
atc4=Classroom(code='LVP7004', name="ATC 4")
atc1.save()
atc2.save()
atc3.save()
atc4.save()


'''CREACIÓN DE DISPOSITIVOS'''
from device.models import Device
print('Generating Device Entities')
dev1=Device(uuid='DEV001', classroom=atc1)
dev2=Device(uuid='DEV002', classroom=atc1)
dev3=Device(uuid='DEV003', classroom=atc2)
dev4=Device(uuid='DEV004', classroom=atc3)
dev1.save()
dev2.save()
dev3.save()
dev4.save()

