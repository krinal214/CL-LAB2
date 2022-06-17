from logging import exception
from sqlite3 import Time
from countrydata import Country
from worlddata import World
from datetime import datetime
from timeline import Timeline
from response import Response
from task1 import retrieve_covid_cases, retrieve_response, retrieve_timeline, retrive_country_urls
from news import News
from nltk.corpus import stopwords
from nltk.tokenize import word_tokenize
from wordcloud import WordCloud, STOPWORDS
import matplotlib.pyplot as plt
import pandas as pd
from collections import Counter
import requests

dict1 = {}  # covid case countries
dict2 = {}  # world data
dict3 = {}  # country data
dict4 = {}  # timeline
dict5 = {}  # response
dict6 = {}  # countrywise news
dict7 = {}  # date range for a country
factor = 0.0001  # small factor to avoid divide by 0

headers = {
    'User-Agent': 'Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/88.0.4324.150 Safari/537.36'
}

def save_log(msg):
    fp = open("logs.txt", "a")
    fp.write(msg)
    fp.close()


def get_data(country, date1, date2, action):

    min_date = min(dict3[country][action].keys())
    max_date = max(dict3[country][action].keys())
    val1, val2 = 0, 0
    if date1 > max_date or date2 > max_date:
        return None, None
    if date1 < min_date:
        val1 = 0
    else:
        val1 = dict3[country][action][date1]
    if date2 < min_date:
        val2 = 0
    else:
        val2 = dict3[country][action][date2]
    return val1, val2

def preprocess(data,iscovid):
    data=data.lower()
    stop_words = set(stopwords.words('english'))
    inclusion=set()
    word_tokens = word_tokenize(data)
    filtered_sentence = [w for w in word_tokens if (not w.lower() in stop_words) and len(w)>2]
    if iscovid:
        file = open("covid_word_dictionary.txt","r")
        arr=file.readlines()
        arr=[i.strip().lower() for i in arr]
        for i in arr:
            set1=set(i.split(" "))
            inclusion=inclusion.union(set1)
        filtered_sentence=[w for w in filtered_sentence if w.lower() in inclusion]
    return filtered_sentence


def count_intersections(lst1, lst2):
    c1 = Counter(lst1)
    c2 = Counter(lst2)
    d={ k: min(c1[k], c2[k]) for k in c1.keys() & c2.keys() }
    return d
def word_cloud(tokens,title="Word cloud"):
    if len(tokens)==0:
        print('Not enough data available to plot word cloud')
        return
    stopwords = set(STOPWORDS)

   
    wordcloud = WordCloud(width = 800, height = 800,
                    background_color ='white',
                    stopwords = stopwords,
                    min_font_size = 10).generate_from_frequencies(tokens)

    # plot the WordCloud image					
    plt.figure(figsize = (8, 8), facecolor = None)
    plt.title(title)
    plt.imshow(wordcloud)
    plt.show()
    


def process_menu4(s1,e1,s2,e2,dictt):
    d1, d2,d3,d4 = None, None,None,None
    try:
        d1 = datetime.strptime(s1, '%d-%m-%Y').date()
        d2 = datetime.strptime(e1, '%d-%m-%Y').date()
        d3 = datetime.strptime(s2, '%d-%m-%Y').date()
        d4 = datetime.strptime(e2, '%d-%m-%Y').date()
    except:
        print("Date is not valid")
        return
    if d1 > d2 or d3>d4:
        print('Start date should be less than end date')
        return
    if (d1>=d3 and d1<=d4) or (d2>=d3 and d2<=d4) or (d3>=d1 and d3<=d2) or (d4>=d1 and d4<=d2):
        print("Given ranges are overlapping")
        return
    text1=''
    text2=''
    for i in sorted (dictt.keys()):
        if i>=d1 and i<=d2:
            text1+=dictt[i]
    for i in sorted (dictt.keys()):
        if i>=d3 and i<=d4:
            text2+=dictt[i]
    data1=preprocess(text1,False)
    data2=preprocess(text2,False)
    data3=preprocess(text1,True)
    data4=preprocess(text2,True)
    dx=count_intersections(data1,data2)
    dy=count_intersections(data3,data4)
    
    print("% of covid words in common words:",(len(dy)+factor)*100/(len(dx)+factor))
    top_20=Counter(dy).most_common(20)
    print("Most common 20 covid words:",top_20)
    word_cloud(dx,'All common words')
    word_cloud(dy,'Only covid related common words')



def process_menu1(country, opt):
    actions = ['Total Cases', 'Active Cases', 'Total Deaths', 'Total Recovered', 'Total Tests',
                'Deaths/M', 'Tests/M', 'New Cases', 'New Deaths', 'New Recovered'
                ]
    val1, val2 = 'NA', 'NA'
    task = actions[opt-1]
    log = country+'\t'+task
    if task in dict2[country].keys():
        val1 = dict2[country][task]
    if val1 != 'NA' and task in dict2['World'].keys():
        val2 = dict2['World'][task]
    if val1 != 'NA' and val2 != 'NA':
        val2 = (val1)*100/(val2)
        val2 = round(val2, 2)
    log += '\t'+str(val1)+'\n'
    save_log(log)
    print(country, ':', task, ' ', val1, '\t', '% of world cases:', val2)


def process_menu2(country, start, end, select1, select2):
    actions = ['Active Cases', 'Daily Deaths', 'New Recovered', 'New Cases']
    date1, date2 = None, None
    try:
        date1 = datetime.strptime(start, '%d-%m-%Y').date()
        date2 = datetime.strptime(end, '%d-%m-%Y').date()
    except:
        print("Date is not valid")
        return
    if date1 > date2:
        print('Start date should be less than end date')
        return
    if select1 < 5:
        if actions[select1-1] not in dict3[country].keys():
            print("Requested data is not available")
            return
        val1, val2 = get_data(country, date1, date2, actions[select1-1])
        if val1 is None or val2 is None:
            print('Date is not valid')
            return
        change = (val2-val1+factor)*100/(val1+factor)
        #print(val2, ' ', val1)
        print('Change in', actions[select1-1], ':', round(change, 2), '%')
    else:
        if actions[select2-1] not in dict3[country].keys():
            print("Requested data is not available")
            return

        val1, val2 = get_data(country, date1, date2, actions[select2-1])
        if val1 is None or val2 is None:
            print('Date is not valid')
            return
        change = (val2-val1+factor)*100/(val1+factor)
        closest = None
        minchange = None
        for c1 in dict1.keys():
            for c2 in dict1[c1]:
                if c2 != country:
                    if actions[select2-1] not in dict3[c2].keys():
                        continue
                    val3, val4 = get_data(c2, date1, date2, actions[select2-1])
                    if val3 is None or val4 is None:
                        continue
                    temp = (val4-val3+factor)*100/(val3+factor)
                    change1 = abs(change-temp)
                    if minchange is None:
                        minchange = change1
                        closest = c2
                    elif change1 < minchange:
                        minchange = change1
                        closest = c2
        if closest is None:
            print("No similar country found for selected criterion")
        else:
            print("Closest country of", country,
                "as per the given criterion:", closest)

def process_menu3(start,end,dictt):
    date1, date2 = None, None
    try:
        date1 = datetime.strptime(start, '%d-%m-%Y').date()
        date2 = datetime.strptime(end, '%d-%m-%Y').date()
    except:
        print("Date is not valid")
        return None
    if date1 > date2:
        print('Start date should be less than end date')
        return None
    ans=''
    for i in sorted (dictt.keys()):
        if i>=date1 and i<=date2:
            ans+=dictt[i]+'\n'
    print(ans)
    return ans

def retrieve_countries(file_name):
    file = open(file_name, "r")
    arr = file.readlines()
    arr = [i.strip() for i in arr]
    continent = None
    countries = {}
    i = 0
    while i < len(arr):
        if continent is None:
            continent = arr[i]
            countries[continent] = []
            i = i+2
        elif arr[i] != '':
            country = arr[i]
            countries[continent].append(country)
            i = i+1
        else:
            continent = None
            i = i+1
    return countries

def jaccard(country,start,end,datax,iscovid):
    date1, date2 = None, None
    try:
        date1 = datetime.strptime(start, '%d-%m-%Y').date()
        date2 = datetime.strptime(end, '%d-%m-%Y').date()
    except:
        print("Date is not valid")
        return None
    if date1 > date2:
        print('Start date should be less than end date')
        return None
     
    list1=[]
    datax=set(datax)
        
    for c in dict6:
        if c==country:
            continue
        ans=''
        for i in sorted (dict6[c].keys()):
            if i>=date1 and i<=date2:
                ans+=dict6[c][i]
        datay=preprocess(ans,iscovid)
        datay=set(datay)
        intersection = datax.intersection(datay)
        union = datax.union(datay)
        list1.append(((len(intersection)+factor)/(len(union)+factor),c))
    list1.sort(reverse=True)
    size=min(3,len(list1))
    print("3 Closest country:",list1[:size])


def init():
    global dict1, dict2, dict3, dict4, dict5, dict6, dict7
    print("Application Initialization Begins!")
    dict1 = retrieve_countries('worldometers_countrylist.txt')
    retrieve_covid_cases(dict1)
    list1 = retrieve_timeline()
    list2 = retrieve_response()
    dict2 = World().crawl('main.html')
    
    for c1 in dict1:
        for c2 in dict1[c1]:
            dict3[c2] = {}
            dict3[c2] = Country().crawl(c2+'.html')
    
    for i in list1:
        print(i[0])
        response=requests.get(url=i[0],headers=headers)
        page_content=response.text
        with open('temp.html', 'w', encoding='utf8') as fp:
            fp.write(page_content)
        Timeline().crawl('temp.html', i[1], i[2], dict4)
        
    for i in list2:
        print(i[0])
        response=requests.get(url=i[0],headers=headers)
        page_content=response.text
        with open('temp.html', 'w', encoding='utf8') as fp:
            fp.write(page_content)
        Response().crawl('temp.html', i[1], i[2], dict5)
        
    
    
    di1 = retrive_country_urls('covid_country_list.txt')
        
    for c in di1:
        dict6[c] = {}
        d1, d2 = None, None
        print(c)
        for j in di1[c]:
            print(j[0])
            response=requests.get(url=j[0],headers=headers)
            page_content=response.text
            with open('temp.html', 'w', encoding='utf8') as fp:
                fp.write(page_content)
            temp = News()
            temp.crawl('temp.html', j[1], j[2], d1, d2, dict6[c])
            if d1 is None:
                d1 = temp.mindate
            elif temp.mindate < d1:
                d1 = temp.mindate
            if d2 is None:
                d2 = temp.maxdate
            elif temp.maxdate > d1:
                d2 = temp.maxdate
        dict7[c] = (d1, d2)
    #print(dict7)
    
    print("Application Initialization Completed!")
    print("------------------------------------------")



def main():
    init()
    choice = {}
    choice['page'] = "home"
    choice['value'] = None

    stack = []

    while(True):
        if choice['page'] == "home":
            stack.clear()
            choice['value'] = None
            print("Home Page")
            print("Press 1 for yesterday's statistics of a country/continent/world")
            print("Press 2 for historical data of a country")
            print("Press 3 for worldwide news between the time range")
            print("Press 4 for worldwide response between the time range")
            print("Press 5 for covid timeline for a country")
            print("Press 6 for word cloud for timeline")
            print("Press 7 for word cloud for response")
            print("Press 8 for exit")
            print("Enter your choice:")
            select = int(input())
            if select == 1:
                stack.append(choice['page'])
                choice['page'] = "yesterday"

            elif select == 2:
                stack.append(choice['page'])
                choice['page'] = "history"
            
            elif select == 3:
                stack.append(choice['page'])
                choice['page']="timeline"
            
            elif select ==4:
                stack.append(choice['page'])
                choice['page']="response"
            
            elif select ==5:
                stack.append(choice['page'])
                choice['page']="country_timeline"
            elif select ==6:
                stack.append(choice['page'])
                choice['page']="cloud_timeline"
            elif select ==7:
                stack.append(choice['page'])
                choice['page']="cloud_response"
            elif select == 8:
                choice['page'] = 'exit'
            else:
                print("You have entered incorrect choice")
        

        elif choice['page']=="cloud_timeline":
            print("Word cloud for two non-overlapping date range(timeline)")
            choice['value']=None
            print("Enter start date for range 1")
            date1=input()
            print("Enter end date for range 1")
            date2=input()
            print("Enter start date for range 2")
            date3=input()
            print("Enter end date for range 2")
            date4=input()
            process_menu4(date1,date2,date3,date4,dict4)
            print("Enter 0 for previous menu")
            print("Enter 1 for home page")
            print("Enter 2 for exit")
            select=int(input())
            if select==0:
                choice['page'] = stack.pop()
            elif select==1:
                choice['page'] = 'home'
            elif select==2:
                choice['page'] = 'exit'
            else:
                print("You have selected incorrect choice")

        elif choice['page']=="cloud_response":
            print("Word cloud for two non-overlapping date range (response)")
            choice['value']=None
            print("Enter start date for range 1")
            date1=input()
            print("Enter end date for range 1")
            date2=input()
            print("Enter start date for range 2")
            date3=input()
            print("Enter end date for range 2")
            date4=input()
            process_menu4(date1,date2,date3,date4,dict5)
            print("Enter 0 for previous menu")
            print("Enter 1 for home page")
            print("Enter 2 for exit")
            select=int(input())
            if select==0:
                choice['page'] = stack.pop()
            elif select==1:
                choice['page'] = 'home'
            elif select==2:
                choice['page'] = 'exit'
            else:
                print("You have selected incorrect choice")


        elif choice['page']=="timeline":
            print("Timeline between date range")
            choice['value']=None
            print("Enter start date")
            date1=input()
            print("Enter end date")
            date2=input()
            process_menu3(date1,date2,dict4)
            print("Enter 0 for previous menu")
            print("Enter 1 for home page")
            print("Enter 2 for exit")
            select=int(input())
            if select==0:
                choice['page'] = stack.pop()
            elif select==1:
                choice['page'] = 'home'
            elif select==2:
                choice['page'] = 'exit'
            else:
                print("You have selected incorrect choice")
        
        elif choice['page']=="response":
            print("Response between date range")
            choice['value']=None
            print("Enter start date")
            date1=input()
            print("Enter end date")
            date2=input()
            process_menu3(date1,date2,dict5)
            print("Enter 0 for previous menu")
            print("Enter 1 for home page")
            print("Enter 2 for exit")
            select=int(input())
            if select==0:
                choice['page'] = stack.pop()
            elif select==1:
                choice['page'] = 'home'
            elif select==2:
                choice['page'] = 'exit'
            else:
                print("You have selected incorrect choice")
        elif choice['page']=="country_timeline":
            print("Time between date range for a country")
            choice['value']=None
            print("Enter country")
            country=input()
            country=country.replace(' ','_')
            if country not in dict6:
                print("Entered country not present")
                continue
            print("Data is available for below time range")
            print(dict7[country][0].strftime('%B %Y'),'-',dict7[country][1].strftime('%B %Y'))
            print("Enter start date")
            date1=input()
            print("Enter end date")
            date2=input()
            dx=process_menu3(date1,date2,dict6[country])
            if dx is None:
                continue
            datax=Counter(preprocess(dx,False))
            word_cloud(datax,'Word cloud')
            print("Enter 0 for previous menu")
            print("Enter 1 for top 3 countries for above selection")
            print("Enter 2 for top 3 countries for above selection (covid words only)")
            print("Enter 3 for home page")
            print("Enter 4 for exit")
            select=int(input())
            if select==0:
                choice['page'] = stack.pop()
            elif select==1:
                jaccard(country,date1,date2,preprocess(dx,False),False)
                print("Enter 0 for previous menu")
                print("Enter 1 for home page")
                print("Enter 2 for exit")
                opt=int(input())
                if opt==0:
                    choice['page'] = stack.pop()
                elif opt==1:
                    choice['page'] = 'home'
                elif opt==2:
                    choice['page'] = 'exit'
                else:
                    print("You have selected incorrect choice")    
            elif select==2:
                jaccard(country,date1,date2,preprocess(dx,True),True)
                print("Enter 0 for previous menu")
                print("Enter 1 for home page")
                print("Enter 2 for exit")
                opt=int(input())
                if opt==0:
                    choice['page'] = stack.pop()
                elif opt==1:
                    choice['page'] = 'home'
                elif opt==2:
                    choice['page'] = 'exit'
                else:
                    print("You have selected incorrect choice")    
            elif select==3:
                choice['page'] = 'home'
            elif select==4:
                choice['page'] = 'exit'
            else:
                print("You have selected incorrect choice")
        elif choice['page'] == "yesterday":
            print("Yesterday's statistics")
            choice['value'] = None
            continents = list(dict1.keys())
            print("Press 0 to go back to previous menu")
            for i in range(0, len(continents)):
                print("Press", i+1, "for", continents[i])
            print("Press", len(continents)+1, "for world")
            print("Press", len(continents)+2, "to go to home page")
            print("Press", len(continents)+3, "to exit")
            select = int(input())
            if select == len(continents)+3:
                choice['page'] = 'exit'
            elif select == len(continents)+2:
                choice['page'] = 'home'
            elif select == len(continents)+1:
                stack.append(choice['page'])
                choice['page'] = "menu1"
                choice['value'] = "World"
            elif select == 0:
                choice['page'] = stack.pop()
            elif select <= len(continents):
                stack.append(choice['page'])
                choice['value'] = continents[select-1]
                countries = dict1[choice['value']]
                print("Press 0 to go back to previous menu")
                for i in range(0, len(countries)):
                    print("Press", i+1, "for", countries[i])
                print("Press", len(countries)+1, "for", choice['value'])
                print("Press", len(countries)+2, "to go to home page")
                print("Press", len(countries)+3, "to exit")
                select = int(input())
                if select == len(countries)+3:
                    choice['page'] = 'exit'
                elif select == len(countries)+2:
                    choice['page'] = 'home'
                elif select == len(countries)+1:
                    stack.append(choice['page'])
                    choice['page'] = "menu1"
                elif select == 0:
                    choice['page'] = stack.pop()
                elif select <= len(countries):
                    choice['page'] = 'menu1'
                    choice['value'] = countries[select-1]
                else:
                    print("You have entered incorrect choice")
            else:
                print("You have entered incorrect choice")

        elif choice['page'] == 'history':
            print("Historical data of a country")
            choice['value'] = None
            continents = list(dict1.keys())
            print("Press 0 to go back to previous menu")
            for i in range(0, len(continents)):
                print("Press", i+1, "to list countries of continent",
                      continents[i])
            print("Press", len(continents)+1, "to go to home page")
            print("Press", len(continents)+2, "to exit")
            select = int(input())
            if select == len(continents)+2:
                choice['page'] = 'exit'
            elif select == len(continents)+1:
                choice['page'] = 'home'
            elif select == 0:
                choice['page'] = stack.pop()
            elif select <= len(continents):
                stack.append(choice['page'])
                choice['value'] = continents[select-1]
                countries = dict1[choice['value']]
                print("Press 0 to go back to previous menu")
                for i in range(0, len(countries)):
                    print("Press", i+1, "for", countries[i])
                print("Press", len(countries)+1, "to go to home page")
                print("Press", len(countries)+2, "to exit")
                select = int(input())
                if select == len(countries)+2:
                    choice['page'] = 'exit'
                elif select == len(countries)+1:
                    choice['page'] = 'home'
                elif select == 0:
                    choice['page'] = stack.pop()
                elif select <= len(countries):
                    choice['page'] = 'menu2'
                    choice['value'] = countries[select-1]
                else:
                    print("You have entered incorrect choice")
            else:
                print("You have entered incorrect choice")

        elif choice['page'] == "menu1":
            print("Yesterday's statistics for:", choice['value'])
            print('Press 0 to go to previous page')
            print('Press 1 for Total Cases')
            print('Press 2 for Active Cases')
            print('Press 3 for Total Deaths')
            print('Press 4 for Total Recovered')
            print('Press 5 for Total Tests')
            print('Press 6 for Deaths/M')
            print('Press 7 for Tests/M')
            print('Press 8 for New Cases')
            print('Press 9 for New Death')
            print('Press 10 for New Recovered')
            print('Presss 11 to go to home page')
            print('Press 12 to exit')
            select = int(input())
            if select == 11:
                choice['page'] = 'home'
            elif select == 12:
                choice['page'] = 'exit'
            elif select == 0:
                choice['page'] = stack.pop()
            elif select <= 10:
                process_menu1(choice['value'], select)
            else:
                print("You have entered incorrect choice")

        elif choice['page'] == "menu2":
            print("Historical Data of:", choice['value'])
            print('Press 0 to go to previous page')
            print('Press 1 for change in Active Cases')
            print('Press 2 for change in Daily Deaths')
            print('Press 3 for change in New Recovered')
            print('Press 4 for change in New Cases')
            print('press 5 for finding similar country')
            print('Presss 6 to go to home page')
            print('Press 7 to exit')
            select = int(input())
            if select == 6:
                choice['page'] = 'home'
            elif select == 7:
                choice['page'] = 'exit'
            elif select == 0:
                choice['page'] = stack.pop()
            elif select < 5:
                print("Enter Start Date(dd-mm-yyyy)")
                date1 = input()
                print("Enter End Date(dd-mm-yyyy)")
                date2 = input()
                process_menu2(choice['value'], date1, date2, select, 0)
            elif select == 5:
                print('Select similarity criterion')
                print('Press 1 for % change in Active Cases')
                print('Press 2 for % change in Daily Deaths')
                print('Press 3 for % change in New Recovered')
                print('Press 4 for % change in New Cases')
                select1 = int(input())
                print("Enter start date(dd-mm-yyyy):")
                date1 = input()
                print("Enter end date(dd-mm-yyyy):")
                date2 = input()
                process_menu2(choice['value'], date1, date2, select, select1)
            else:
                print("You have entered incorrect choice")

        else:
            stack.clear()
            print("Exiting from the application")
            break

        print("--------------------------------------------------------------")


if __name__ == "__main__":
    main()
