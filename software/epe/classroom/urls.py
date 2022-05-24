from django.urls import path
from .views import *

urlpatterns = [
    path('', list),
    path('create/', create),
    path('<str:id>/', detail),
    path('<str:id>/update/', update),
    path('<str:id>/delete/', delete),
]
