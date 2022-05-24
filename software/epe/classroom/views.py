from django.shortcuts import render, redirect
from epe import settings
from .models import Classroom
from epe.utils import *

def list(request):
    if not request.user.is_authenticated:
        return redirect('%s?next=%s' % (settings.LOGIN_REDIRECT_URL, request.path))
    classrooms = Classroom.objects.all()
    return render(request, 'list.html', {'classrooms': classrooms, 'request': request})

def detail(request, id):
    if not request.user.is_authenticated:
        return redirect('%s?next=%s' % (settings.LOGIN_REDIRECT_URL, request.path))
    classroom = Classroom.objects.get(code=id)
    return render(request, 'detail.html', {'classroom': classroom})

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
            return render(request, 'create.html', {'classroom': classroom, 'error': "Código del aula no introducido"})
        if not name:
            return render(request, 'create.html', {'classroom': classroom, 'error': "Nombre del aula no introducido"})
        if Classroom.objects.filter(code=code).exists():
            return render(request, 'create.html', {'classroom': classroom, 'error': "Código del aula ya existe"})
        classroom.save()
        request.session['info'] = "Aula agregada"
        return render(request, "detail.html", {'classroom': classroom, 'info': "Aula agregada"})
    return render(request, 'create.html')

def update(request, id):
    if not request.user.is_authenticated:
        return redirect('%s?next=%s' % (settings.LOGIN_REDIRECT_URL, request.path))
    classroom = Classroom.objects.get(code=id)
    if request.POST:
        code = request.POST['code']
        name = request.POST['name']
        if not code:
            return render(request, 'update.html', {'classroom': classroom, 'error': "Código del aula no introducido"})
        if not name:
            return render(request, 'update.html', {'classroom': classroom, 'error': "Nombre del aula no introducido"})
        if classroom.code != code and Classroom.objects.filter(code=code).exists():
            return render(request, 'update.html', {'classroom': classroom, 'error': "Código del aula ya existe"})
        classroom.code = code
        classroom.name = name
        classroom.save()
        return render(request, 'detail.html', {'classroom': classroom, 'info': "Aula "+classroom.name+" actualizada correctamente"})
    return render(request, 'update.html', {'classroom': classroom})

def delete(request, id):
    if not request.user.is_authenticated:
        return redirect('%s?next=%s' % (settings.LOGIN_REDIRECT_URL, request.path))
    classroom = Classroom.objects.get(code=id)
    classroom.delete()
    classrooms = Classroom.objects.all()
    return render(request, 'list.html', {'classrooms': classrooms, 'info': "Aula "+classroom.name+" borrada"})
