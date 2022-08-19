import http

from django.contrib.auth.models import User
from django.shortcuts import render, redirect

from epe import settings
from .models import Classroom
from epe.utils import *
from django.core.exceptions import PermissionDenied, ObjectDoesNotExist


def list(request):
    if not request.user.is_authenticated:
        return redirect('%s?next=%s' % (settings.LOGIN_REDIRECT_URL, request.path))
    if request.user.is_staff:
        classrooms = Classroom.objects.all()
    else:
        classrooms = request.user.classrooms.all()
    return render(request, 'classroom_list.html', {'classrooms': classrooms})

def detail(request, id):
    if not request.user.is_authenticated:
        return redirect('%s?next=%s' % (settings.LOGIN_REDIRECT_URL, request.path))
    try:
        classroom = Classroom.objects.get(code=id)
    except ObjectDoesNotExist:
        return render(request, 'errors/404.html', {'error': 'El Aula no existe'})
    if not request.user.is_staff and request.user not in classroom.teachers.all():
        return render(request, 'errors/403.html', {'error': 'Usted no está esta acreditado como profesor en esta aula'})
    return render(request, 'classroom_detail.html', {'classroom': classroom, 'devices': classroom.devices.all().filter(token__isnull=False), 'documents': classroom.documents.all()})

def create(request):
    if not request.user.is_authenticated:
        return redirect('%s?next=%s' % (settings.LOGIN_REDIRECT_URL, request.path))
    if not request.user.is_staff:
        return render(request, 'errors/403.html', {'error': 'Usted no está acreditado para crear aulas'})
    classroom = Classroom()
    if request.POST:
        code = request.POST['code']
        name = request.POST['name']
        teachers = request.POST['teachers']
        classroom.code = code
        classroom.name = name
        if not code:
            return render(request, 'classroom_create.html', {'classroom': classroom, 'teachers': User.objects.all(), 'error': "Código del aula no introducido"})
        if not name:
            return render(request, 'classroom_create.html', {'classroom': classroom, 'teachers': User.objects.all(), 'error': "Nombre del aula no introducido"})
        if Classroom.objects.filter(code=code).exists():
            return render(request, 'classroom_create.html', {'classroom': classroom, 'teachers': User.objects.all(), 'error': "Código del aula ya existe"})
        classroom.save()
        request.session['info'] = "Aula agregada"
        return render(request, "classroom_detail.html", {'classroom': classroom, 'info': "Aula agregada"})
    return render(request, 'classroom_create.html', {'teachers': User.objects.all()})

def update(request, id):
    if not request.user.is_authenticated:
        return redirect('%s?next=%s' % (settings.LOGIN_REDIRECT_URL, request.path))
    if not request.user.is_staff:
        return render(request, 'errors/403.html', {'error': 'Usted no está esta acreditado para modificar aulas'})
    classroom = Classroom.objects.get(code=id)
    if request.POST:
        name = request.POST['name']
        teachers = request.POST.getlist('teachers')
        classroom.teachers.clear()
        for teacher in teachers:
            classroom.teachers.add(User.objects.get(id=teacher))
        if not name:
            return render(request, 'classroom_update.html', {'classroom': classroom, 'teachers': User.objects.all(), 'error': "Nombre del aula no introducido"})
        classroom.name = name
        classroom.save()
        return render(request, 'classroom_detail.html', {'classroom': classroom, 'teachers': User.objects.all(), 'info': "Aula " + classroom.name + " actualizada correctamente"})
    return render(request, 'classroom_update.html', {'classroom': classroom, 'teachers': User.objects.all()})

def delete(request, id):
    if not request.user.is_authenticated:
        return redirect('%s?next=%s' % (settings.LOGIN_REDIRECT_URL, request.path))
    if not request.user.is_staff:
        return render(request, 'errors/403.html', {'error': 'No esta acreditado para borrar aulas'})
    classroom = Classroom.objects.get(code=id)
    if classroom.devices.all().filter(token__isnull=False).count() > 0:
        return render(request, 'classroom_detail.html', {'classroom': classroom, 'devices': classroom.devices.all().filter(token__isnull=False), 'error': "El aula " + classroom.name + " tiene dispositivos asociados"})
    classroom.delete()
    classrooms = Classroom.objects.all()
    return render(request, 'classroom_list.html', {'classrooms': classrooms, 'info': "Aula " + classroom.name + " borrada"})
