import datetime

from django.contrib.auth.models import User
from django.shortcuts import render, redirect
from pip._internal import req

from classroom.models import Classroom
from epe import settings
from .models import Document

def list(request):
    if not request.user.is_authenticated:
        return redirect('%s?next=%s' % (settings.LOGIN_REDIRECT_URL, request.path))
    if request.user.is_staff:
        documents = Document.objects.all()
    else:
        documents = []
        for classroom in request.user.classrooms.all():
            documents += classroom.documents.filter(owner=request.user)
    return render(request, 'document_list.html', {'documents': documents})

def detail(request, id):
    if not request.user.is_authenticated:
        return redirect('%s?next=%s' % (settings.LOGIN_REDIRECT_URL, request.path))
    try:
        document = Document.objects.get(id=id)
    except Document.DoesNotExist:
        return render(request, 'errors/404.html', {'error': 'El documento no existe'})
    if not request.user.is_staff and request.user.id is not document.owner_id:
        return render(request, 'errors/403.html', {'error': 'Usted no está acreditado para ver este documento'})
    return render(request, 'document_detail.html', {'document': document})

def create(request):
    if not request.user.is_authenticated:
        return redirect('%s?next=%s' % (settings.LOGIN_REDIRECT_URL, request.path))
    document = Document()
    document.start = datetime.datetime.now()
    document.end = datetime.datetime.now()+datetime.timedelta(hours=2)
    if request.user.is_staff:
        classrooms = Classroom.objects.all()
    else:
        classrooms = request.user.classrooms.all()
    if request.POST:
        name = request.POST['name']
        description = request.POST['description']
        classroom = request.POST['classroom']
        subject_code = request.POST['subject_code']
        start = request.POST['start']
        end = request.POST['end']
        owner = request.POST['owner']
        document.description = description
        if not name:
            return render(request, 'document_create.html', {'document': document, 'classrooms': classrooms, 'users': User.objects.all(), 'error': "Nombre del documento no introducido"})
        document.name = name
        if not subject_code:
            return render(request, 'document_create.html', {'document': document, 'classrooms': classrooms, 'users': User.objects.all(), 'error': "Código de la asignatura no introducido"})
        document.subject_code = subject_code
        if not classroom or not Classroom.objects.filter(code=classroom).exists():
            return render(request, 'document_create.html', {'document': document, 'classrooms': classrooms, 'users': User.objects.all(), 'error': "Aula asignada no introducida"})
        document.classroom = Classroom.objects.get(code=classroom)
        if not request.user.is_staff and document.classroom not in request.user.classrooms.all():
            return render(request, 'document_create.html', {'document': document, 'classrooms': classrooms, 'users': User.objects.all(), 'error': 'Usted no está acreditado crear este documento en el aula seleccionada'})
        document.owner = User.objects.get(id=owner)
        if not start:
            return render(request, 'document_create.html', {'document': document, 'classrooms': classrooms, 'users': User.objects.all(), 'error': "Fecha de inicio no introducida"})
        start_parsed = start.split(' ')
        if len(start_parsed) != 2 or len(start_parsed[0]) == 0 or len(start_parsed[1]) == 0:
            return render(request, 'document_create.html', {'document': document, 'classrooms': classrooms, 'users': User.objects.all(), 'error': "La fecha de inicio no tiene el formato correcto (HH:MM DD/MM/YYYY)"})
        start_parsed_date = start_parsed[1].split('/')
        if len(start_parsed_date) != 3 or len(start_parsed_date[0]) == 0 or len(start_parsed_date[1]) == 0 or len(
                start_parsed_date[2]) == 0:
            return render(request, 'document_create.html', {'document': document, 'classrooms': classrooms, 'users': User.objects.all(), 'error': "La fecha de inicio no tiene el formato correcto (HH:MM DD/MM/YYYY)"})
        start_parsed_time = start_parsed[0].split(':')
        if len(start_parsed_time) != 2 or len(start_parsed_time[0]) == 0 or len(start_parsed_time[1]) == 0:
            return render(request, 'document_create.html', {'document': document, 'classrooms': classrooms, 'users': User.objects.all(), 'error': "La fecha de inicio no tiene el formato correcto (HH:MM DD/MM/YYYY)"})
        document.start = datetime.datetime(int(start_parsed_date[2]), int(start_parsed_date[1]), int(start_parsed_date[0]), int(start_parsed_time[0]), int(start_parsed_time[1]), 0, 0)

        if not end:
            return render(request, 'document_create.html', {'document': document, 'classrooms': classrooms, 'users': User.objects.all(), 'error': "Fecha de finalización no introducida"})
        end_parsed = end.split(' ')
        if len(end_parsed) != 2 or len(end_parsed[0]) == 0 or len(end_parsed[1]) == 0:
            return render(request, 'document_create.html', {'document': document, 'classrooms': classrooms, 'users': User.objects.all(), 'error': "La fecha de finalización no tiene el formato correcto (HH:MM DD/MM/YYYY)"})
        end_parsed_time = end_parsed[0].split(':')
        if len(end_parsed_time) != 2 or len(end_parsed_time[0]) == 0 or len(end_parsed_time[1]) == 0:
            return render(request, 'document_create.html', {'document': document, 'classrooms': classrooms, 'users': User.objects.all(), 'error': "La fecha de finalización no tiene el formato correcto (HH:MM DD/MM/YYYY)"})
        end_parsed_date = end_parsed[1].split('/')
        if len(end_parsed_date) != 3 or len(end_parsed_date[0]) == 0 or len(end_parsed_date[1]) == 0 or len(end_parsed_date[2]) == 0:
            return render(request, 'document_create.html', {'document': document, 'classrooms': classrooms, 'users': User.objects.all(), 'error': "La fecha de finalización no tiene el formato correcto (HH:MM DD/MM/YYYY)"})
        document.end = datetime.datetime(int(end_parsed_date[2]), int(end_parsed_date[1]), int(end_parsed_date[0]), int(end_parsed_time[0]), int(end_parsed_time[1]), 0, 0)

        document.save()
        return render(request, 'document_detail.html', {'document': document, 'info': "Nuevo documento guardado"})
    return render(request, 'document_create.html', {'document': document, 'classrooms': classrooms, 'users': User.objects.all()})

def update(request, id):
    if not request.user.is_authenticated:
        return redirect('%s?next=%s' % (settings.LOGIN_REDIRECT_URL, request.path))
    try:
        document = Document.objects.get(id=id)
    except Document.DoesNotExist:
        return render(request, 'errors/404.html', {'error': 'El documento no existe'})
    if request.user.is_staff:
        classrooms = Classroom.objects.all()
    else:
        classrooms = request.user.classrooms.all()
    if not request.user.is_staff and request.user.id is not document.owner_id:
        return render(request, 'errors/403.html', {'error': 'Usted no está acreditado para modificar este documento'})
    if request.POST:
        name = request.POST['name']
        description = request.POST['description']
        classroom = request.POST['classroom']
        subject_code = request.POST['subject_code']
        start = request.POST['start']
        end = request.POST['end']
        owner = request.POST['owner']

        document.description = description
        document.owner = User.objects.get(id=owner)
        if not name:
            return render(request, 'document_update.html', {'document': document, 'classrooms': classrooms, 'users': User.objects.all(), 'error': "Nombre del documento no introducido"})
        document.name = name
        if not subject_code:
            return render(request, 'document_update.html', {'document': document, 'classrooms': classrooms, 'users': User.objects.all(), 'error': "Código de la asignatura no introducido"})
        document.subject_code = subject_code
        if not classroom or not Classroom.objects.filter(code=classroom).exists():
            return render(request, 'document_update.html', {'document': document, 'classrooms': classrooms, 'users': User.objects.all(), 'error': "Aula asignada no introducida"})
        document.classroom = Classroom.objects.get(code=classroom)
        if not request.user.is_staff and document.classroom not in request.user.classrooms.all():
            return render(request, 'document_create.html', {'document': document, 'classrooms': classrooms, 'users': User.objects.all(), 'error': 'Usted no está acreditado crear este documento en el aula seleccionada'})
        if not start:
            return render(request, 'document_update.html', {'document': document, 'classrooms': classrooms, 'users': User.objects.all(), 'error': "Fecha de inicio no introducida"})
        start_parsed = start.split(' ')
        if len(start_parsed) != 2 or len(start_parsed[0]) == 0 or len(start_parsed[1]) == 0:
            return render(request, 'document_update.html', {'document': document, 'classrooms': classrooms, 'users': User.objects.all(), 'error': "La fecha de inicio no tiene el formato correcto (HH:MM DD/MM/YYYY)"})
        start_parsed_date = start_parsed[1].split('/')
        if len(start_parsed_date) != 3 or len(start_parsed_date[0]) == 0 or len(start_parsed_date[1]) == 0 or len(start_parsed_date[2]) == 0:
            return render(request, 'document_update.html', {'document': document, 'classrooms': classrooms, 'users': User.objects.all(), 'error': "La fecha de inicio no tiene el formato correcto (HH:MM DD/MM/YYYY)"})
        start_parsed_time = start_parsed[0].split(':')
        if len(start_parsed_time) != 2 or len(start_parsed_time[0]) == 0 or len(start_parsed_time[1]) == 0:
            return render(request, 'document_update.html', {'document': document, 'classrooms': classrooms, 'users': User.objects.all(), 'error': "La fecha de inicio no tiene el formato correcto (HH:MM DD/MM/YYYY)"})
        document.start = datetime.datetime(int(start_parsed_date[2]), int(start_parsed_date[1]), int(start_parsed_date[0]), int(start_parsed_time[0]), int(start_parsed_time[1]),0,0)

        if not end:
            return render(request, 'document_update.html', {'document': document, 'classrooms': classrooms, 'users': User.objects.all(), 'error': "Fecha de finalización no introducida"})
        end_parsed = end.split(' ')
        if len(end_parsed) != 2 or len(end_parsed[0]) == 0 or len(end_parsed[1]) == 0:
            return render(request, 'document_update.html', {'document': document, 'classrooms': classrooms, 'users': User.objects.all(), 'error': "La fecha de finalización no tiene el formato correcto (HH:MM DD/MM/YYYY)"})
        end_parsed_time = end_parsed[0].split(':')
        if len(end_parsed_time) != 2  or len(end_parsed_time[0]) == 0 or len(end_parsed_time[1]) == 0:
            return render(request, 'document_update.html', {'document': document, 'classrooms': classrooms, 'users': User.objects.all(), 'error': "La fecha de finalización no tiene el formato correcto (HH:MM DD/MM/YYYY)"})
        end_parsed_date = end_parsed[1].split('/')
        if len(end_parsed_date) != 3 or len(end_parsed_date[0]) == 0 or len(end_parsed_date[1]) == 0 or len(end_parsed_date[2]) == 0:
            return render(request, 'document_update.html', {'document': document, 'classrooms': classrooms, 'users': User.objects.all(), 'error': "La fecha de finalización no tiene el formato correcto (HH:MM DD/MM/YYYY)"})
        document.end = datetime.datetime(int(end_parsed_date[2]), int(end_parsed_date[1]), int(end_parsed_date[0]), int(end_parsed_time[0]), int(end_parsed_time[1]),0,0)

        document.save()
        return render(request, 'document_detail.html', {'document': document, 'classrooms': classrooms, 'info': "Documento guardado"})
    return render(request, 'document_update.html', {'document': document, 'classrooms': classrooms, 'users': User.objects.all()})

def delete(request, id):
    if not request.user.is_authenticated:
        return redirect('%s?next=%s' % (settings.LOGIN_REDIRECT_URL, request.path))
    try:
        document = Document.objects.get(id=id)
    except Document.DoesNotExist:
        return render(request, 'errors/404.html', {'error': 'El documento a borrar no existe'})
    if not request.user.is_staff and request.user.id is not document.owner_id:
        return render(request, 'errors/403.html', {'error': 'Usted no está acreditado para borrar este documento'})
    document.delete()
    if request.user.is_staff:
        documents = Document.objects.all()
    else:
        documents = []
        for classroom in request.user.classrooms.all():
            documents += classroom.documents.all()
    return render(request, 'document_list.html', {'documents': documents, 'info': "documento borrado"})
