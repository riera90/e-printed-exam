from datetime import timedelta

from django.utils import timezone
import pytz
from django.http import HttpResponse
from django.shortcuts import render, redirect
import json

from django.views.decorators.csrf import csrf_exempt

from classroom.models import Classroom
from epe import settings
from .models import Device, vendors
from epe.utils import *
from rest_framework.views import APIView
from epe.secrets import server_secret

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
    if request.POST:
        if not request.POST.get('uuid'):
            return render(request, 'device_create.html', {"error": "El dispositivo selecionado no consta en la base de datos"})
        uuid = request.POST['uuid']
        try:
            device = Device.objects.get(uuid=uuid)
            if device.token:
                return render(request, 'device_create.html', {"error": "El dispositivo selecionado ya está registrado, actualice la página para ver dispositivos nuevos"})
            device.last_seen = timezone.now()
            device.token = "TOKEN"
            device.name = "Nuevo dispositivo"
            device.classroom = Classroom.objects.all()[0]
            device.save()
            return redirect("/device/"+device.uuid+"/update/")
        except Device.DoesNotExist:
            return render(request, 'device_create.html', {"error": "El dispositivo selecionado no consta en la base de datos"})

    return render(request, 'device_create.html')


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


def web_api_search_device(request):
    if not request.user.is_authenticated:
        return HttpResponse(json.dumps({"error": "auth error"}), content_type="application/json")
    devices = Device.objects.filter(classroom__isnull=True)
    devices_json = []
    for device in devices:
        devices_json.append({"uuid": device.uuid})
    response = {"devices": devices_json}
    return HttpResponse(json.dumps(response), content_type="application/json")

@csrf_exempt
def device_api_acknowledgment_device(request):
    ttl = 3600
    print(request.POST)
    if not request.method == "POST":
        return HttpResponse(json.dumps({"error": "only post requests"}), content_type="application/json")
    if not request.POST.get('secret'):
        return HttpResponse(json.dumps({"error": "only requests with secret attribute"}), content_type="application/json")
    print(request.POST['secret'])
    if request.POST['secret'] != server_secret:
        return HttpResponse(json.dumps({"error": "auth error"}), content_type="application/json")
    if not request.POST.get('uuid'):
        return HttpResponse(json.dumps({"error": "parse error, no uuid in post"}), content_type="application/json")
    if not request.POST.get('vendor'):
        return HttpResponse(json.dumps({"error": "parse error, no vendor in post"}), content_type="application/json")
    Device.objects.filter(last_seen__lt=(timezone.now()-timedelta(seconds=ttl)), token__isnull=True).delete()
    uuid = request.POST['uuid']
    vendor = request.POST['vendor']

    if vendor not in vendors:
        return HttpResponse(json.dumps({"error": "invalid vendor"}), content_type="application/json")

    try:
        device = Device.objects.get(uuid=uuid)
        device.ttl = ttl
        device.last_seen = timezone.now()
        device.save()
    except Device.DoesNotExist:
        device = Device(uuid=uuid, vendor=vendor, last_seen=timezone.now(), ttl=ttl)
        device.save()

    return HttpResponse(json.dumps({"ttl": ttl, "token": device.token}), content_type="application/json")
