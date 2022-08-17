import datetime
from datetime import timedelta

from PIL import Image
from django.utils import timezone
import pytz
from django.http import HttpResponse
from django.shortcuts import render, redirect
import json

from django.views.decorators.csrf import csrf_exempt

from classroom.models import Classroom
from document.models import Document
from epe import settings
from page.models import Page
from .models import Device, vendors
from epe.utils import *
from rest_framework.views import APIView
from epe.secrets import server_secret


def list(request):
    if not request.user.is_authenticated:
        return redirect('%s?next=%s' % (settings.LOGIN_REDIRECT_URL, request.path))
    if request.user.is_staff:
        devices = Device.objects.all().filter(token__isnull=False)
    else:
        devices = []
        for classroom in request.user.classrooms.all():
            devices += classroom.devices.all()
    return render(request, 'device_list.html', {'devices': devices})


def detail(request, id):
    if not request.user.is_authenticated:
        return redirect('%s?next=%s' % (settings.LOGIN_REDIRECT_URL, request.path))
    try:
        device = Device.objects.get(uuid=id)
    except Device.DoesNotExist:
        return render(request, 'errors/404.html', {'error': 'Este dispositivo no existe'})
    if not request.user.is_staff and not request.user in device.classroom.teachers.all():
        return render(request, 'errors/403.html', {'error': 'Usted no está acreditado para ver este dispositivo'})
    return render(request, 'device_detail.html', {'device': device})


def create(request):
    if not request.user.is_authenticated:
        return redirect('%s?next=%s' % (settings.LOGIN_REDIRECT_URL, request.path))
    if not request.user.is_staff:
        return render(request, 'errors/403.html', {'error': 'Usted no está acreditado para crear dispositivos'})
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
    if not request.user.is_staff:
        return render(request, 'errors/403.html', {'error': 'Usted no está acreditado para modificar dispositivos'})
    device = Device.objects.get(uuid=id)
    classrooms = Classroom.objects.all()
    if request.POST:
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
    if not request.user.is_staff:
        return render(request, 'errors/403.html', {'error': 'Usted no está acreditado para borrar dispositivos'})
    device = Device.objects.get(uuid=id)
    device.delete()
    devices = Device.objects.all()
    return render(request, 'device_list.html', {'devices': devices, 'info': "Dispositivo " + str(device.name) + " borrado"})


def web_api_search_device(request):
    if not request.user.is_authenticated or not request.user.is_staff:
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
    if not request.method == "POST":
        return HttpResponse(json.dumps({"error": "only post requests"}), content_type="application/json")
    if not request.POST.get('secret'):
        return HttpResponse(json.dumps({"error": "only requests with secret attribute"}), content_type="application/json")
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
    document_pages = 0
    documents = device.classroom.documents.all().filter(start__lt=datetime.datetime.now(), end__gt=datetime.datetime.now())
    if documents and len(documents) > 0:
        document_pages = documents[0].pages.all().count()
    return HttpResponse(json.dumps({"ttl": str(ttl), "token": device.token, "document_pages": str(document_pages)}), content_type="application/json")


@csrf_exempt
def device_api_get_page(request):
    if not request.method == "POST":
        return HttpResponse(json.dumps({"error": "only post requests"}), content_type="application/json")
    if not request.POST.get('secret'):
        return HttpResponse(json.dumps({"error": "only requests with secret attribute"}), content_type="application/json")
    if request.POST.get('secret') != server_secret:
        return HttpResponse(json.dumps({"error": "invalid secret"}), content_type="application/json")
    if not request.POST.get('token'):
        return HttpResponse(json.dumps({"error": "only requests with token attribute"}), content_type="application/json")
    if not request.POST.get('uuid'):
        return HttpResponse(json.dumps({"error": "parse error, no uuid in post"}), content_type="application/json")
    if not request.POST.get('page'):
        return HttpResponse(json.dumps({"error": "parse error, no page in post"}), content_type="application/json")
    uuid = request.POST.get('uuid')
    token = request.POST.get('token')
    Device.objects.get(uuid=uuid)
    uuid = request.POST['uuid']
    page = int(request.POST['page'])
    try:
        device = Device.objects.get(uuid=uuid)
    except Device.DoesNotExist:
        return HttpResponse(json.dumps({"error": "invalid uuid"}), content_type="application/json")
    if device.token != token:
        return HttpResponse(json.dumps({"error": "invalid token"}), content_type="application/json")
    document = Document.objects.get(id=1)
    try:
        page = document.pages.get(order=page)
    except Page.DoesNotExist:
        return HttpResponse(json.dumps({"page_error": "document has no page "+str(page)}), content_type="application/json")

    if page.type == "I":
        image = Image.frombytes('L', (page.w, page.h), page.binary_image)
        image_bits = ""
        for y in range(page.h):
            for x in range(page.w):
                if image.getpixel((x,y)):
                    image_bits+='1'
                else:
                    image_bits+='0'
        return HttpResponse(json.dumps({"type": page.type, "data": image_bits}), content_type="application/json")
    return HttpResponse(json.dumps({"type": page.type, "title": page.title, "body": page.body+"\r\n"}), content_type="application/json")
