import datetime

from django.contrib.auth import authenticate, login, logout
from django.contrib.auth.models import User
from django.http import HttpResponseRedirect, HttpResponse, HttpResponseForbidden, HttpResponseNotFound, HttpResponseServerError
from django.shortcuts import render, redirect
from django.template.loader import render_to_string

from document.models import Document
from epe import settings


def index(request):
    if not request.user.is_authenticated:
        return redirect('%s?next=%s' % (settings.LOGIN_REDIRECT_URL, request.path))
    if request.user.is_staff:
        documents_next = Document.objects.filter(start__gt=datetime.datetime.now())
    else:
        documents_next = []
        for classroom in request.user.classrooms.all():
            documents_next += classroom.documents.filter(start__gt=datetime.datetime.now(), owner=request.user)
    if request.user.is_staff:
        documents_current = Document.objects.filter(start__lt=datetime.datetime.now(), end__gt=datetime.datetime.now())
    else:
        documents_current = []
        for classroom in request.user.classrooms.all():
            documents_current += classroom.documents.filter(start__lt=datetime.datetime.now(), end__gt=datetime.datetime.now(), owner=request.user)
    return render(request, 'index.html', {'documents_next': documents_next, 'documents_current': documents_current})

def login_handler(request):
    logout(request)
    if request.POST:
        username = request.POST['user']
        password = request.POST['password']
        next = request.POST['next']
        user = authenticate(username=username, password=password)
        if user is not None:
            if user.is_active:
                request.session.set_expiry(86400)  # sets the exp. value of the session
                login(request, user)
                return HttpResponseRedirect(next)
        else:
            return render(request, 'login.html', {'next': next, 'error': "Usuario o contraseña no válidos"})

    #return render_to_response('login.html', context_instance=RequestContext(request))
    else:
        next = request.GET['next']
        return render(request, 'login.html', {'next': next})


def logout_handler(request):
    logout(request)
    return HttpResponseRedirect('/')


def error403(request, exception=None):
    return HttpResponseForbidden(render_to_string('errors/403.html', {}))


def error404(request, exception=None):
    return HttpResponseNotFound(render_to_string('errors/404.html', {}))


def error500(request, exception=None):
    return HttpResponseServerError(render_to_string('errors/500.html', {}))