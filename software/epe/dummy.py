from datetime import time
import random

from PIL import Image
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
dev1=Device(vendor="novendor", uuid='DEV001', classroom=atc1, name="DEVICE 1", ttl=3600)
dev2=Device(vendor="novendor", uuid='DEV002', classroom=atc1, name="DEVICE 2", ttl=3600)
dev3=Device(vendor="novendor", uuid='DEV003', classroom=atc2, name="DEVICE 3", ttl=3600)
dev4=Device(vendor="novendor", uuid='DEV004', classroom=atc3, name="DEVICE 4", ttl=3600)
dev1.save()
dev2.save()
dev3.save()
dev4.save()

'''CREACIÓN DE DOCUMENTOS'''
from document.models import Document
print('Generating Document Entities')
doctest=Document(name="prueba tecnica", description="prueba de sistema completa", subject_code="TEST0001", classroom=atc1, start=datetime.now(), end=datetime.now())
doc1=Document(name="DOCUMENT 1", description="Descripción del documento 1", subject_code="SUBJ001", classroom=atc1, start=datetime.now(), end=datetime.now())
doc2=Document(name="DOCUMENT 2", description="Descripción del documento 2", subject_code="SUBJ002", classroom=atc1, start=datetime.now(), end=datetime.now())
doc3=Document(name="DOCUMENT 3", description="Descripción del documento 3", subject_code="SUBJ003", classroom=atc2, start=datetime.now(), end=datetime.now())
doc4=Document(name="DOCUMENT 4", description="Descripción del documento 4", subject_code="SUBJ004", classroom=atc2, start=datetime.now(), end=datetime.now())
doctest.save()
doc1.save()
doc2.save()
doc3.save()
doc4.save()


'''CREACIÓN DE PÁGINAS'''
from page.models import Page
print('Generating Page Entities')
p1=Page(document=doctest, order=1, type="T", title="Test texto", body='''Esto es una prueba de texto para las
diapositivas, está intencionado para 
rellenar lo maximo posible de la pantalla
lo mismo está hasta demasiado llena.

Pero bueno, lo importante es que esto
funcione correctamente en el display
de tinta electrónica.

Espero que la cantidad de texto que la
pantalla es capaz de acomodar sea
suficiente para una pregunta de examen
de universidad.

Aunque el sistema tenga limitaciones creo
que este sera suficiente para  funcion.

Atentamente, firmado. 0x4455434B''')
p1.save()

image = Image.open("media/spider.png")
bwimage = image.convert("L")
bwimage = bwimage.point(lambda p: 255 if p > 140 else 0)
p2=Page(document=doctest, order=2, type="I", h=bwimage.size[1], w=bwimage.size[0], binary_image=bwimage.tobytes())
p2.save()
image.close()
bwimage.close()
