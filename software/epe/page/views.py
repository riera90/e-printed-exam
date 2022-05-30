import os
import time

from django.conf import settings
from django.core.files.storage import FileSystemStorage
from django.shortcuts import render, redirect
from .models import Page, Types
from document.models import Document
from PIL import Image
from threading import Thread

# Create your views here.


def document_page_list(request, id):
    if not request.user.is_authenticated:
        return redirect('%s?next=%s' % (settings.LOGIN_REDIRECT_URL, request.path))
    document = Document.objects.get(id=id)
    pages = document.pages.order_by('order').all()
    pages_lines=[]
    for page in pages:
        if len(page.body.splitlines()) > 0:
            pages_lines.append(page.body.splitlines())
        else:
            pages_lines.append([])
        if page.type == "I":
            save_image_half(page)
    zipped_pages = zip(pages, pages_lines)
    zipped_pages2 = zip(pages, pages_lines)
    return render(request, 'document_page_list.html', {'pages': zipped_pages, 'pages2': zipped_pages2, 'document': document})


def remove_file(filename, delay_seconds):
    time.sleep(delay_seconds)
    os.system("rm -rf "+filename)


def save_image_half(page):
    if not page.binary_image:
        return
    image = Image.frombytes('L', (page.w, page.h), page.binary_image)
    if image.size[0] != 200:
        index = 200.0 / float(image.size[0])
        image = image.resize((200, int(image.size[1] * index)))
    if image.size[1] != 150:
        index = 150.0 / float(image.size[1])
        if index < 1:
            image = image.resize((int(image.size[0] * index), 150))
    image.save("media/snd/"+str(page.id)+"_half.jpg")
    #image.show()
    rm_thd = Thread(target=remove_file, args=("./media/snd/"+str(page.id)+"_half.jpg", 5, ))
    rm_thd.start()

def save_image(page):
    if not page.binary_image:
        return
    image = Image.frombytes('L', (page.w, page.h), page.binary_image)
    image.save("media/snd/"+str(page.id)+".jpg")
    #image.show()
    rm_thd = Thread(target=remove_file, args=("./media/snd/"+str(page.id)+".jpg", 5, ))
    rm_thd.start()

def document_page_update(request, id, page_order):
    if not request.user.is_authenticated:
        return redirect('%s?next=%s' % (settings.LOGIN_REDIRECT_URL, request.path))
    document = Document.objects.get(id=id)
    page = document.pages.get(order=page_order)
    if request.POST:
        if request.POST.get('type') and request.POST['type'] != page.type:
            page.type = request.POST['type']
            page.save()
            save_image(page)
            return render(request, 'document_page_update.html', {'page': page, 'body_lines': page.body.splitlines(), 'document': document, 'types': Types, 'info': "Página cambiada a modo "+("imagen" if request.POST['type'] == "I" else "texto")})
        if page.type == "I":
            if not request.FILES.get("image"):
                save_image(page)
                return render(request, 'document_page_update.html', {'page': page, 'body_lines': page.body.splitlines(), 'document': document, 'types': Types, 'error': "Imagen no encontrada"})
            if not request.POST.get("threshold"):
                save_image(page)
                return render(request, 'document_page_update.html', {'page': page, 'body_lines': page.body.splitlines(), 'document': document, 'types': Types, 'error': "parametro de binarizacion no encontrado"})
            image = request.FILES['image']
            threshold = int(request.POST['threshold'])
            fss = FileSystemStorage()
            file = fss.save("rcv/"+str(page.id)+".jpg", image)
            image = Image.open("media/rcv/"+str(page.id)+".jpg")
            print(image)
            if image.size[0] != 400:
                index = 400.0 / float(image.size[0])
                image = image.resize((400, int(image.size[1]*index)))
            if image.size[1] != 300:
                index = 300.0 / float(image.size[1])
                if index < 1:
                    image = image.resize((int(image.size[0] * index), 300))
            bwimage = image.convert("L")
            bwimage = bwimage.point(lambda p: 255 if p > threshold else 0)
            #bwimage.show()
            page.binary_image = bwimage.tobytes()
            page.w = bwimage.size[0]
            page.h = bwimage.size[1]
            page.save()
            image.close()
            bwimage.close()
            os.system("rm -rf ./media/rcv/"+str(page.id)+".jpg")
        if page.type == "T" and request.POST.get('title'):
            page.title = request.POST['title']
            page.save()
        if page.type == "T" and request.POST.get('title'):
            page.body = request.POST['body']
            page.save()
        if page.type == "I":
            save_image(page)
        return redirect("/document/" + str(document.id) + "/page/")
    if page.type == "I":
        save_image(page)
    return render(request, 'document_page_update.html', {'page': page, 'body_lines': page.body.splitlines(), 'document': document, 'types': Types})

def document_page_create(request, id):
    document = Document.objects.get(id=id)
    if document.pages.all().exists():
        page = Page(
            document=document,
            order=document.pages.all().order_by('order').last().order+1
        )
    else:
        page = Page(
            document=document,
            order=1
        )
    page.save()
    return redirect("/document/"+str(document.id)+"/page/"+str(page.order))

def document_page_down(request, id, page_order):
    document = Document.objects.get(id=id)
    page = document.pages.all().get(order=page_order)
    page_2 = document.pages.all().get(order=page_order+1)
    page.order = page.order + 1
    page_2.order = page_2.order - 1
    page.save()
    page_2.save()
    return redirect("/document/"+str(document.id)+"/page/")

def document_page_up(request, id, page_order):
    document = Document.objects.get(id=id)
    page = document.pages.all().get(order=page_order)
    page_2 = document.pages.all().get(order=page_order - 1)
    page.order = page.order - 1
    page_2.order = page_2.order + 1
    page.save()
    page_2.save()
    return redirect("/document/" + str(document.id) + "/page/")

def document_page_delete(request, id, page_order):
    document = Document.objects.get(id=id)
    target_page = document.pages.all().get(order=page_order)
    pages = document.pages.all()
    for page in pages:
        if page.order > target_page.order:
            page.order = page.order - 1
            page.save()
    target_page.delete()
    return redirect("/document/" + str(document.id) + "/page/")