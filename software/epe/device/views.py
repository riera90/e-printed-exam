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
        name = request.POST['name']
        if not name:
            return render(request, 'device_update.html', {'device': device, 'classrooms': classrooms, 'error': "Nombre del dispositivo no introducido"})
        device.name = name
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
