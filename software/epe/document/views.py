import datetime

from django.shortcuts import render, redirect
from classroom.models import Classroom
from epe import settings
from .models import Document

def list(request):
    if not request.user.is_authenticated:
        return redirect('%s?next=%s' % (settings.LOGIN_REDIRECT_URL, request.path))
    documents = Document.objects.all()
    return render(request, 'document_list.html', {'documents': documents})

def detail(request, id):
    if not request.user.is_authenticated:
        return redirect('%s?next=%s' % (settings.LOGIN_REDIRECT_URL, request.path))
    document = Document.objects.get(id=id)
    return render(request, 'document_detail.html', {'document': document})

def create(request):
    if not request.user.is_authenticated:
        return redirect('%s?next=%s' % (settings.LOGIN_REDIRECT_URL, request.path))
    document = Document()
    document.start = datetime.datetime.now()
    document.end = datetime.datetime.now()
    classrooms = Classroom.objects.all()
    if request.POST:
        name = request.POST['name']
        description = request.POST['description']
        classroom = request.POST['classroom']
        subject_code = request.POST['subject_code']
        start = request.POST['start']
        end = request.POST['end']
        document.description = description
        if not name:
            return render(request, 'document_create.html', {'document': document, 'classrooms': classrooms, 'error': "Nombre del documento no introducido"})
        document.name = name
        if not subject_code:
            return render(request, 'document_create.html', {'document': document, 'classrooms': classrooms, 'error': "Código de la asignatura no introducido"})
        document.subject_code = subject_code
        if not classroom or not Classroom.objects.filter(code=classroom).exists():
            return render(request, 'document_create.html', {'document': document, 'classrooms': classrooms, 'error': "Aula asignada no introducida"})
        document.classroom = Classroom.objects.get(code=classroom)
        if not start:
            return render(request, 'document_create.html', {'document': document, 'classrooms': classrooms, 'error': "Fecha de inicio no introducida"})
        start_parsed = start.split(' ')
        if len(start_parsed) != 2 or len(start_parsed[0]) == 0 or len(start_parsed[1]) == 0:
            return render(request, 'document_create.html', {'document': document, 'classrooms': classrooms, 'error': "La fecha de inicio no tiene el formato correcto (HH:MM DD/MM/YYYY)"})
        start_parsed_date = start_parsed[1].split('/')
        if len(start_parsed_date) != 3 or len(start_parsed_date[0]) == 0 or len(start_parsed_date[1]) == 0 or len(
                start_parsed_date[2]) == 0:
            return render(request, 'document_create.html', {'document': document, 'classrooms': classrooms, 'error': "La fecha de inicio no tiene el formato correcto (HH:MM DD/MM/YYYY)"})
        start_parsed_time = start_parsed[0].split(':')
        if len(start_parsed_time) != 2 or len(start_parsed_time[0]) == 0 or len(start_parsed_time[1]) == 0:
            return render(request, 'document_create.html', {'document': document, 'classrooms': classrooms, 'error': "La fecha de inicio no tiene el formato correcto (HH:MM DD/MM/YYYY)"})
        document.start = datetime.datetime(int(start_parsed_date[2]), int(start_parsed_date[1]), int(start_parsed_date[0]), int(start_parsed_time[0]), int(start_parsed_time[1]), 0, 0)

        if not end:
            return render(request, 'document_create.html', {'document': document, 'classrooms': classrooms, 'error': "Fecha de finalización no introducida"})
        end_parsed = end.split(' ')
        if len(end_parsed) != 2 or len(end_parsed[0]) == 0 or len(end_parsed[1]) == 0:
            return render(request, 'document_create.html', {'document': document, 'classrooms': classrooms, 'error': "La fecha de finalización no tiene el formato correcto (HH:MM DD/MM/YYYY)"})
        end_parsed_time = end_parsed[0].split(':')
        if len(end_parsed_time) != 2 or len(end_parsed_time[0]) == 0 or len(end_parsed_time[1]) == 0:
            return render(request, 'document_create.html', {'document': document, 'classrooms': classrooms, 'error': "La fecha de finalización no tiene el formato correcto (HH:MM DD/MM/YYYY)"})
        end_parsed_date = end_parsed[1].split('/')
        if len(end_parsed_date) != 3 or len(end_parsed_date[0]) == 0 or len(end_parsed_date[1]) == 0 or len(end_parsed_date[2]) == 0:
            return render(request, 'document_create.html', {'document': document, 'classrooms': classrooms, 'error': "La fecha de finalización no tiene el formato correcto (HH:MM DD/MM/YYYY)"})
        document.end = datetime.datetime(int(end_parsed_date[2]), int(end_parsed_date[1]), int(end_parsed_date[0]), int(end_parsed_time[0]), int(end_parsed_time[1]), 0, 0)

        document.save()
        return render(request, 'document_detail.html', {'document': document, 'info': "Nuevo documento guardado"})
    return render(request, 'document_create.html', {'document': document, 'classrooms': classrooms})

def update(request, id):
    if not request.user.is_authenticated:
        return redirect('%s?next=%s' % (settings.LOGIN_REDIRECT_URL, request.path))
    document = Document.objects.get(id=id)
    classrooms = Classroom.objects.all()
    if request.POST:
        name = request.POST['name']
        description = request.POST['description']
        classroom = request.POST['classroom']
        subject_code = request.POST['subject_code']
        start = request.POST['start']
        end = request.POST['end']

        document.description = description
        if not name:
            return render(request, 'document_update.html', {'document': document, 'classrooms': classrooms, 'error': "Nombre del documento no introducido"})
        document.name = name
        if not subject_code:
            return render(request, 'document_update.html', {'document': document, 'classrooms': classrooms, 'error': "Código de la asignatura no introducido"})
        document.subject_code = subject_code
        if not classroom or not Classroom.objects.filter(code=classroom).exists():
            return render(request, 'document_update.html', {'document': document, 'classrooms': classrooms, 'error': "Aula asignada no introducida"})
        document.classroom = Classroom.objects.get(code=classroom)
        if not start:
            return render(request, 'document_update.html', {'document': document, 'classrooms': classrooms, 'error': "Fecha de inicio no introducida"})
        start_parsed = start.split(' ')
        if len(start_parsed) != 2 or len(start_parsed[0]) == 0 or len(start_parsed[1]) == 0:
            return render(request, 'document_update.html', {'document': document, 'classrooms': classrooms, 'error': "La fecha de inicio no tiene el formato correcto (HH:MM DD/MM/YYYY)"})
        start_parsed_date = start_parsed[1].split('/')
        if len(start_parsed_date) != 3 or len(start_parsed_date[0]) == 0 or len(start_parsed_date[1]) == 0 or len(start_parsed_date[2]) == 0:
            return render(request, 'document_update.html', {'document': document, 'classrooms': classrooms, 'error': "La fecha de inicio no tiene el formato correcto (HH:MM DD/MM/YYYY)"})
        start_parsed_time = start_parsed[0].split(':')
        if len(start_parsed_time) != 2 or len(start_parsed_time[0]) == 0 or len(start_parsed_time[1]) == 0:
            return render(request, 'document_update.html', {'document': document, 'classrooms': classrooms, 'error': "La fecha de inicio no tiene el formato correcto (HH:MM DD/MM/YYYY)"})
        document.start = datetime.datetime(int(start_parsed_date[2]), int(start_parsed_date[1]), int(start_parsed_date[0]), int(start_parsed_time[0]), int(start_parsed_time[1]),0,0)

        if not end:
            return render(request, 'document_update.html', {'document': document, 'classrooms': classrooms, 'error': "Fecha de finalización no introducida"})
        end_parsed = end.split(' ')
        if len(end_parsed) != 2 or len(end_parsed[0]) == 0 or len(end_parsed[1]) == 0:
            return render(request, 'document_update.html', {'document': document, 'classrooms': classrooms, 'error': "La fecha de finalización no tiene el formato correcto (HH:MM DD/MM/YYYY)"})
        end_parsed_time = end_parsed[0].split(':')
        if len(end_parsed_time) != 2  or len(end_parsed_time[0]) == 0 or len(end_parsed_time[1]) == 0:
            return render(request, 'document_update.html', {'document': document, 'classrooms': classrooms, 'error': "La fecha de finalización no tiene el formato correcto (HH:MM DD/MM/YYYY)"})
        end_parsed_date = end_parsed[1].split('/')
        if len(end_parsed_date) != 3 or len(end_parsed_date[0]) == 0 or len(end_parsed_date[1]) == 0 or len(end_parsed_date[2]) == 0:
            return render(request, 'document_update.html', {'document': document, 'classrooms': classrooms, 'error': "La fecha de finalización no tiene el formato correcto (HH:MM DD/MM/YYYY)"})
        document.end = datetime.datetime(int(end_parsed_date[2]), int(end_parsed_date[1]), int(end_parsed_date[0]), int(end_parsed_time[0]), int(end_parsed_time[1]),0,0)

        document.save()
        return render(request, 'document_detail.html', {'document': document, 'classrooms': classrooms, 'info': "Documento guardado"})
    return render(request, 'document_update.html', {'document': document, 'classrooms': classrooms})

def delete(request, id):
    if not request.user.is_authenticated:
        return redirect('%s?next=%s' % (settings.LOGIN_REDIRECT_URL, request.path))
    Document.objects.get(id=id).delete()
    documents = Document.objects.all()
    return render(request, 'document_list.html', {'documents': documents, 'info': "documento borrado"})
