from importlib.metadata import entry_points
import requests

entrypoint = 'https://www.worldometers.info/coronavirus/'
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
def retrieve_webpage(countries):
    response = requests.get(url=entrypoint, headers=headers)
    page_content = response.text
    with open('main.html', 'w', encoding='utf8') as fp:
        fp.write(page_content)
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
    print("Web pages for all the countries have been retrived successfully")

def main():
    countries=retrieve_countries('worldometers_countrylist.txt')
    print(countries)
    retrieve_webpage(countries)

if __name__ == "__main__":
    main()
