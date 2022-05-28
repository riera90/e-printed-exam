from django.shortcuts import render, redirect

from classroom.models import Classroom
from epe import settings
from .models import Device
from epe.utils import *

def list(request):
    if not request.user.is_authenticated:
        return redirect('%s?next=%s' % (settings.LOGIN_REDIRECT_URL, request.path))
    devices = Device.objects.all()
    return render(request, 'device_list.html', {'devices': devices})

def detail(request, id):
    if not request.user.is_authenticated:
        return redirect('%s?next=%s' % (settings.LOGIN_REDIRECT_URL, request.path))
    device = Device.objects.get(uuid=id)
    return render(request, 'device_detail.html', {'device': device})

def create(request):
    if not request.user.is_authenticated:
        return redirect('%s?next=%s' % (settings.LOGIN_REDIRECT_URL, request.path))
    classrooms = Classroom.objects.all()
    if request.POST:
        pass
    return render(request, 'device_create.html', {'classrooms': classrooms})

def update(request, id):
    if not request.user.is_authenticated:
        return redirect('%s?next=%s' % (settings.LOGIN_REDIRECT_URL, request.path))
    device = Device.objects.get(uuid=id)
    classrooms = Classroom.objects.all()
    if request.POST:
        print(request.POST)
        name = request.POST['name']
        classroom_id = request.POST['classroom']
        if not name:
            return render(request, 'device_update.html', {'device': device, 'classrooms': classrooms, 'error': "Nombre del dispositivo no introducido"})
        if not classroom_id or not Classroom.objects.filter(code=classroom_id).exists():
            return render(request, 'device_update.html', {'device': device, 'classrooms': classrooms, 'error': "Aula asignada no introducida"})
        device.name = name
        device.classroom = Classroom.objects.get(code=classroom_id)
        device.save()
        return render(request, 'device_detail.html', {'device': device, 'classrooms': classrooms, 'info': "Dispositivo " + device.name + " actualizado correctamente"})
    return render(request, 'device_update.html', {'device': device, 'classrooms': classrooms})

def delete(request, id):
    if not request.user.is_authenticated:
        return redirect('%s?next=%s' % (settings.LOGIN_REDIRECT_URL, request.path))
    device = Device.objects.get(uuid=id)
    device.delete()
    devices = Device.objects.all()
    return render(request, 'device_list.html', {'devices': devices, 'info': "Dispositivo " + device.name + " borrado"})
