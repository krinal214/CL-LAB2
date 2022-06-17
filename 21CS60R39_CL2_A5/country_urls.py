
import ply.yacc as yacc
import ply.lex as lex
import re


class CountryUrl:
    def __init__(self):
        self.dict1 = {}
        self.countries=None
        self.tokens = [
            'START',
            'HREF',
            'END'
        ]
    def t_START(self,t):
        r'<span.class="mw-headline".id="Timeline_by_country"'
        return t
    def t_END(self,t):
        r'<span.class="mw-headline".id="Worldwide_cases_by_month_and_year"'
        return t
    def t_HREF(self,t):
        r'<a.href="\/wiki\/Timeline_of_the_COVID-19_pandemic_[a-zA-Z0-9_()–%"?\/\-= ]*'
        return t
   
    t_ignore = " \t"

    def t_error(self, t):
        t.lexer.skip(1)
    
    def p_error(self, t):
        pass

    def p_start(self,t):
        'start : START url END'

    def p_url(self,t):
        '''url : HREF url
                | HREF'''
        url=t[1].split('"')
        for c in self.countries:
            temp=re.findall(c,url[1])
            if len(temp)>0:
                if c not in self.dict1:
                    self.dict1[c]=[]
                if (url[1],None,None) not in self.dict1[c]:
                    self.dict1[c].append((url[1],None,None))
            
    
    
        
    def crawl(self, file_name,countries):
        text = open(file_name, 'r', errors='ignore').read()
        self.countries=countries
        self.lexer = lex.lex(object=self)
        self.lexer.input(str(text))
        #for tok in self.lexer:
        #  print(tok.type, tok.value)
        self.parser = yacc.yacc(module=self)
        self.parser.parse(text)
        return self.dict1

# c=CountryUrl()
# c.crawl('timeline.html',['India'])
# print(c.dict1)
