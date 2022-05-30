from django.conf.urls.static import static
from django.urls import path, include
from .views import *

urlpatterns = [
    path('', document_page_list),
    path('create/', document_page_create),
    path('<int:page_order>/', document_page_update),
    path('<int:page_order>/delete/', document_page_delete),
    path('<int:page_order>/up/', document_page_up),
    path('<int:page_order>/down/', document_page_down),
]
