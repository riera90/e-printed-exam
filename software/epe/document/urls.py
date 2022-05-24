from django.urls import path
from .views import *

urlpatterns = [
    path('', list),
    path('create/', create),
    path('<id:id>/', detail),
    path('<id:id>/update/', update),
    path('<id:id>/delete/', delete),
]
