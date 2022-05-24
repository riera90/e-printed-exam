from django.contrib.auth import authenticate, login, logout
from django.contrib.auth.models import User
from django.http import HttpResponseRedirect, HttpResponse
from django.shortcuts import render, redirect

from epe import settings


def index(request):
    if not request.user.is_authenticated:
        return redirect('%s?next=%s' % (settings.LOGIN_REDIRECT_URL, request.path))
    return render(request, 'index.html', {})

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
