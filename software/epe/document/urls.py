from django.urls import path, include
from .views import *

urlpatterns = [
    path('', list),
    path('create/', create),
    path('<int:id>/', detail),
    path('<int:id>/page/', include('page.urls')),
    path('<int:id>/update/', update),
    path('<int:id>/delete/', delete),
]
