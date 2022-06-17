from logging import exception
from countrydata import Country
from worlddata import World
from datetime import datetime


dict1 = {}
dict2 = {}
dict3 = {}
factor = 0.0001  # small factor to avoid divide by 0


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


def process_menu1(country, opt):
    actions = ['Total Cases', 'Active Cases', 'Total Deaths', 'Total Recovered', 'Total Tests',
               'Deaths/M', 'Tests/M', 'New Cases', 'New Deaths', 'New Recovered'
               ]
    val1, val2 = 'NA', 'NA'
    task = actions[opt-1]
    log = country+'\t'+task
    if task in dict2[country].keys():
        val1 = dict2[country][task]
    if val1!='NA' and task in dict2['World'].keys():
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


def init():
    global dict1, dict2, dict3
    print("Application Initialization Begins!")
    dict1 = retrieve_countries('worldometers_countrylist.txt')
    # print(dict1)
    dict2 = World().crawl('main.html')

    for c1 in dict1.keys():
        for c2 in dict1[c1]:
            dict3[c2] = {}
            dict3[c2] = Country().crawl(c2+'.html')
            #print(c2,' ',len(dict3[c2].keys()))

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
            print("Press 3 for exit")
            print("Enter your choice:")
            select = int(input())
            if select == 1:
                stack.append(choice['page'])
                choice['page'] = "yesterday"

            elif select == 2:
                stack.append(choice['page'])
                choice['page'] = "history"
            elif select == 3:
                choice['page'] = 'exit'
            else:
                print("You have entered incorrect choice")

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
