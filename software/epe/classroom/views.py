from django.shortcuts import render, redirect
from epe import settings
from .models import Classroom
from epe.utils import *

def list(request):
    if not request.user.is_authenticated:
        return redirect('%s?next=%s' % (settings.LOGIN_REDIRECT_URL, request.path))
    classrooms = Classroom.objects.all()
    return render(request, 'classroom_list.html', {'classrooms': classrooms})

def detail(request, id):
    if not request.user.is_authenticated:
        return redirect('%s?next=%s' % (settings.LOGIN_REDIRECT_URL, request.path))
    classroom = Classroom.objects.get(code=id)
    return render(request, 'classroom_detail.html', {'classroom': classroom, 'devices': classroom.devices.all()})

def create(request):
    if not request.user.is_authenticated:
        return redirect('%s?next=%s' % (settings.LOGIN_REDIRECT_URL, request.path))
    classroom = Classroom()
    if request.POST:
        code = request.POST['code']
        name = request.POST['name']
        classroom.code = code
        classroom.name = name
        if not code:
            return render(request, 'classroom_create.html', {'classroom': classroom, 'error': "Código del aula no introducido"})
        if not name:
            return render(request, 'classroom_create.html', {'classroom': classroom, 'error': "Nombre del aula no introducido"})
        if Classroom.objects.filter(code=code).exists():
            return render(request, 'classroom_create.html', {'classroom': classroom, 'error': "Código del aula ya existe"})
        classroom.save()
        request.session['info'] = "Aula agregada"
        return render(request, "classroom_detail.html", {'classroom': classroom, 'info': "Aula agregada"})
    return render(request, 'classroom_create.html')

def update(request, id):
    if not request.user.is_authenticated:
        return redirect('%s?next=%s' % (settings.LOGIN_REDIRECT_URL, request.path))
    classroom = Classroom.objects.get(code=id)
    if request.POST:
        name = request.POST['name']
        if not name:
            return render(request, 'classroom_update.html', {'classroom': classroom, 'error': "Nombre del aula no introducido"})
        classroom.name = name
        classroom.save()
        return render(request, 'classroom_detail.html', {'classroom': classroom, 'info': "Aula " + classroom.name + " actualizada correctamente"})
    return render(request, 'classroom_update.html', {'classroom': classroom})

def delete(request, id):
    if not request.user.is_authenticated:
        return redirect('%s?next=%s' % (settings.LOGIN_REDIRECT_URL, request.path))
    classroom = Classroom.objects.get(code=id)
    if classroom.devices.all().count() > 0:
        return render(request, 'classroom_detail.html', {'classroom': classroom, 'error': "El aula " + classroom.name + " tiene dispositivos asociados"})
    classroom.delete()
    classrooms = Classroom.objects.all()
    return render(request, 'classroom_list.html', {'classrooms': classrooms, 'info': "Aula " + classroom.name + " borrada"})
