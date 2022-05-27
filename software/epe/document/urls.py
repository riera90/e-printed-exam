from django.urls import path
from .views import *

urlpatterns = [
    path('', list),
    path('create/', create),
    path('<int:id>/', detail),
    path('<int:id>/update/', update),
    path('<int:id>/delete/', delete),
]
