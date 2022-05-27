from django.shortcuts import render, redirect
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
    if request.POST:
        return render(request, 'document_detail.html', {'document': document, 'info': "Nuevo documento guardado"})
    return render(request, 'document_create.html')

def update(request, id):
    if not request.user.is_authenticated:
        return redirect('%s?next=%s' % (settings.LOGIN_REDIRECT_URL, request.path))
    document = Document.objects.get(id=id)
    if request.POST:
        return render(request, 'document_detail.html', {'document': document, 'info': "Documento guardado"})
    return render(request, '../templates/classroom_update.html', {'document': document})

def delete(request, id):
    if not request.user.is_authenticated:
        return redirect('%s?next=%s' % (settings.LOGIN_REDIRECT_URL, request.path))
    Document.objects.get(id=id).delete()
    documents = Document.objects.all()
    return render(request, 'document_list.html', {'documents': documents, 'info': "documento borrado"})
