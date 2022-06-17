from calendar import month
import requests
from country_urls import CountryUrl
import re


headers = {
    'User-Agent': 'Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/88.0.4324.150 Safari/537.36'
}
#utility function to read the list of countries from a file 
def retrieve_countries(file_name):
    file = open(file_name,"r")
    arr=file.readlines()
    arr=[i.strip() for i in arr]
    continent=None
    countries={}
    i=0
    while i < len(arr):
        if continent is None:
            continent=arr[i]
            countries[continent]=[]
            i=i+2
        elif arr[i]!='':
            country=arr[i]
            countries[continent].append(country)
            i=i+1
        else:
            continent=None
            i=i+1
    return countries


#utility function to download webpages for all the countries
def retrieve_covid_cases(countries):
    entrypoint='https://www.worldometers.info/coronavirus/'
    response = requests.get(url=entrypoint, headers=headers)
    page_content = response.text
    with open('main.html', 'w', encoding='utf8') as fp:
        fp.write(page_content)
    list1=[]
    for _,country in countries.items():
        for c in country:
            link=c.lower().replace(' ','-')
            if link=='usa':
                link='us'
            if link=='vietnam':
                link='viet-nam'
            urls=entrypoint+'country/'+link
            response=requests.get(url=urls,headers=headers)
            page_content=response.text
            with open(c+'.html', 'w', encoding='utf8') as fp:
                fp.write(page_content)
            list1.append(c+'.html')
    print("Web pages for all the countries have been retrived successfully")
    

def retrieve_timeline():
    entrypoint = 'https://en.wikipedia.org/wiki/Timeline_of_the_COVID-19_pandemic'
    response = requests.get(url=entrypoint, headers=headers)
    page_content = response.text
    months=['January','February','March','April','May','June','July','August','September','October','November','December']
    years=['2020','2021','2022']

    with open('timeline.html', 'w', encoding='utf8') as fp:
        fp.write(page_content)
    list1=[]
    for y in years:
        for m in months:
            urls=entrypoint+'_in_'+m+'_'+y
            fname=y+'_'+m+'_timeline.html'
            list1.append((urls,y,m))
            if (y=='2022' and m=='April'):
                break
            
    return list1

def retrieve_response():
    entrypoint = 'https://en.wikipedia.org/wiki/Responses_to_the_COVID-19_pandemic'
    months=['January','February','March','April','May','June','July','August','September','October','November','December']
    years=['2020','2021','2022']
    list1=[]
    for y in years:
        for m in months:
            urls=entrypoint+'_in_'+m+'_'+y
            fname=y+'_'+m+'_response.html'
            list1.append((urls,y,m))
            if y=='2022' and m=='April':
                break
            
    return list1

def retrive_country_urls(file_name):
    file = open(file_name,"r")
    arr=file.readlines()
    arr=[i.strip() for i in arr]
    arr=[i.replace(' ','_') for i in arr]
    print(arr)
    entrypoint='https://en.wikipedia.org/wiki/Timeline_of_the_COVID-19_pandemic'
    response=requests.get(url=entrypoint,headers=headers)
    page_content=response.text
    with open('timeline.html', 'w', encoding='utf8') as fp:
        fp.write(page_content)
    years=['2019','2020','2021','2022']
    dict1=CountryUrl().crawl('timeline.html',arr)
    home_url='https://en.wikipedia.org'
    for key in dict1:
        for v in range(len(dict1[key])):
            url=dict1[key][v][0]
            dict1[key][v]=(home_url+dict1[key][v][0],'2020',None)
            if key=='Canada':
                dict1[key][v]=(dict1[key][v][0],'2019',None)
            for k in years:
                if re.search(k,dict1[key][v][0]):
                    dict1[key][v]=(dict1[key][v][0],k,k)
                    break
    return dict1
