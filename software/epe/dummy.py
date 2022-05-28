from datetime import time
import random

from django.contrib.auth.hashers import make_password
from django.utils.timezone import datetime


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
dev1=Device(uuid='DEV001', classroom=atc1, name="DEVICE 1")
dev2=Device(uuid='DEV002', classroom=atc1, name="DEVICE 2")
dev3=Device(uuid='DEV003', classroom=atc2, name="DEVICE 3")
dev4=Device(uuid='DEV004', classroom=atc3, name="DEVICE 4")
dev1.save()
dev2.save()
dev3.save()
dev4.save()

'''CREACIÓN DE DOCUMENTOS'''
from document.models import Document
print('Generating Document Entities')
doc1=Document(name="DOCUMENT 1", description="Descripción del documento 1", subject_code="SUBJ001", classroom=atc1, start=datetime.now(), end=datetime.now())
doc2=Document(name="DOCUMENT 2", description="Descripción del documento 2", subject_code="SUBJ002", classroom=atc1, start=datetime.now(), end=datetime.now())
doc3=Document(name="DOCUMENT 3", description="Descripción del documento 3", subject_code="SUBJ003", classroom=atc2, start=datetime.now(), end=datetime.now())
doc4=Document(name="DOCUMENT 4", description="Descripción del documento 4", subject_code="SUBJ004", classroom=atc2, start=datetime.now(), end=datetime.now())
doc1.save()
doc2.save()
doc3.save()
doc4.save()