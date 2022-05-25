from django.shortcuts import render, redirect
from epe import settings
from .models import Document

def list(request):
    if not request.user.is_authenticated:
        return redirect('%s?next=%s' % (settings.LOGIN_REDIRECT_URL, request.path))
    documents = Document.objects.all()
    return render(request, '../templates/classroom_list.html', {'documents': documents})

def detail(request, id):
    if not request.user.is_authenticated:
        return redirect('%s?next=%s' % (settings.LOGIN_REDIRECT_URL, request.path))
    document = Document.objects.get(id=id)
    return render(request, '../templates/classroom_detail.html', {'document': document})

def create(request):
    if not request.user.is_authenticated:
        return redirect('%s?next=%s' % (settings.LOGIN_REDIRECT_URL, request.path))
    classroom = Document()
    if request.POST:
        code = request.POST['code']
        name = request.POST['name']
        classroom.code = code
        classroom.name = name
        if not code:
            return render(request, '../templates/classroom_create.html', {'classroom': classroom, 'error': "Código del aula no introducido"})
        if not name:
            return render(request, '../templates/classroom_create.html', {'classroom': classroom, 'error': "Nombre del aula no introducido"})
        if Document.objects.filter(id=id).exists():
            return render(request, '../templates/classroom_create.html', {'classroom': classroom, 'error': "Código del aula ya existe"})
        classroom.save()
        return render(request, '../templates/classroom_detail.html', {'classroom': classroom, 'info': "Nueva Aula guardada"})
    return render(request, '../templates/classroom_create.html')

def update(request, id):
    if not request.user.is_authenticated:
        return redirect('%s?next=%s' % (settings.LOGIN_REDIRECT_URL, request.path))
    document = Document.objects.get(id=id)
    if request.POST:
        code = request.POST['code']
        name = request.POST['name']
        if not code:
            return render(request, '../templates/classroom_update.html', {'document': document, 'error': "Código del aula no introducido"})
        if not name:
            return render(request, '../templates/classroom_update.html', {'document': document, 'error': "Nombre del aula no introducido"})
        if document.code != code and Document.objects.filter(id=code).exists():
            return render(request, '../templates/classroom_update.html', {'document': document, 'error': "Código del aula ya existe"})
        document.code = code
        document.name = name
        document.save()
        return render(request, '../templates/classroom_detail.html', {'document': document, 'info': "Aula guardada"})
    return render(request, '../templates/classroom_update.html', {'document': document})

def delete(request, id):
    if not request.user.is_authenticated:
        return redirect('%s?next=%s' % (settings.LOGIN_REDIRECT_URL, request.path))
    Document.objects.get(id=id).delete()
    documents = Document.objects.all()
    return render(request, '../templates/classroom_list.html', {'documents': documents, 'info': "documento borrado"})
