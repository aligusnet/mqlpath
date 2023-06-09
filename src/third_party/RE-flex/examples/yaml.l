// A fast YAML 1.2 parser and writer
// Written by Robert van Engelen
//
// https://yaml.org/spec/1.2/spec.html
//
// YAML doesn't define a formal grammar but instead defines over 200 rules.
// YAML uses indent to define structure.  This RE/flex lexer and parser uses
// indentation anchors \i, \j, and \k to parse YAML structures.
//
// This YAML parser follows the specification but does not generate errors for
// invalid YAML syntax, just tries to make sense of it all (YAML is complex!).
//
// Note:
// - directives are ignored
// - anchors (&id and *id) are stored with the YAML structure, but not resolved
// - tags (!tag) are parsed and stored with the YAML structure, but not written
// - scalars are always stored as strings (not converted to numbers/Booleans)
//
// YAML tokens generated by the lexer:
// - 'S' YAML document start marker ---
// - 'E' YAML document end marker ...
// - ';' newline, i.e. \r?\n
// - '=' one or more empty or blank lines
// - '$' string (a YAML scalar, quoted scalar, or block scalar)
// - '-' sequence dash
// - '?' map key
// - ':' map colon
// - '>' indent after ';' or '='
// - '<' dedent after ';' or '='
// - '[' flow sequence open bracket
// - ']' flow sequence close bracket
// - '{' flow sequence open brace
// - '}' flow sequence close brace
// - ',' flow sequence or map comma
//
// YAML test files:
// - https://www.genivia.com/files/yamltests.zip

%top{
  #include <stdlib.h> // strtoul()
  #include <iostream> // std::cout etc.
  #include <iomanip>  // std::setw
  #include <vector>   // to store YAML containers
}

%{
  // #define SHOW_TOKENS // to output tokens for debugging
%}

// Lexer class members
%class{

 public:

  // wide string to accumulate YAML scalars
  std::wstring string;

 protected:

  // count number of newlines matched
  size_t newlines()
  {
    return chr() == '\r' ? size()/2 : size();
  }

  // parse the indent value given after a '|' or '>', if present
  void parse_indent(size_t offset)
  {
    indent = strtoul(text() + offset, NULL, 10);
  }

  // use the parsed indent value given after a '|' or '>' to adjust the indent
  void adjust_indent()
  {
    if (indent > 0)
    {
      std::vector<size_t>& stops = matcher().stops();
      size_t spaces = stops.back();
      if (spaces > indent)
      {
        stops.pop_back();
        if (stops.empty())
        {
          stops.push_back(indent);
        }
        else
        {
          spaces -= stops.back();
          stops.push_back(stops.back() + indent);
        }
        string.append(spaces - indent, L' ');
      }
    }
  }

  // clear the string scalar before accumulating a new scalar
  void clear()
  {
    string.clear();
  }

  // add one or n chars c to the string
  void add(wchar_t c, size_t n = 1)
  {
    while (n-- > 0)
      string.push_back(c);
  }

  // add indent to the string, prefixed with a \n if nl is true
  void add_indent()
  {
    if (nl)
      string.push_back(L'\n');
    size_t stop = matcher().last_stop();
    if (size() > stop)
      string.append(size() - stop, L' ');
  }

  // if nl is true, add a \n to the string then reset nl
  void add_newline()
  {
    if (nl)
    {
      string.push_back(L'\n');
      nl = false;
    }
  }

  // add sp spaces to the string
  void add_space()
  {
    string.append(sp, L' ');
    sp = 0;
  }

  // chomp the string
  void chomp()
  {
    switch (mode)
    {
      case CLIP:
        while (!string.empty() && string.back() == L'\n')
          string.pop_back();
        string.push_back(L'\n');
        break;
      case STRIP:
        while (!string.empty() && string.back() == L'\n')
          string.pop_back();
        break;
      case KEEP:
        break;
    }
  }

  unsigned long indent;            // block scalar indent value
  size_t sp;                       // insert spaces in folded block scalar
  bool nl;                         // insert newline in folded block scalar
  enum { CLIP, STRIP, KEEP } mode; // chomp mode
}

// Lexer class initialization at construction
%init{
  indent = 0;
  nl = false;
  sp = 0;
  mode = CLIP;
}

%o fast freespace dotall unicode

%x APOS QUOT PRES FOLD PBLK FBLK

direct  \h* % [^\n]* \n
comment \h* # [^\n]*
ic      [^-?:\\\[\]{},!&*#'"@`[:space:]]
rc      [^\\\[\]{}:,[:space:]]
rd      [^-\\\[\]{}:,[:space:]]
rh      [^#\\\[\]{}:,[:space:]]
scalar  ({ic} | [-:?] {rd} | --- {rc}) ({rc} | :+ {rc} | \h+ {rh})*
tag     [!&*] {rc}+
h2      [[:xdigit:]]{2}
h4      [[:xdigit:]]{4}
h8      [[:xdigit:]]{8}
nl      \h* (# [^\n]* | \r)? \n
lf      \r? \n
bl      {lf} (\h* {lf})+
br      \h+ | (\h* {lf})+

%%

{direct}        { /* ignore directive */ }
{comment}       { /* ignore comment */ }
\h* {lf}        { return ';'; }
\h* {bl}        { return '='; }
^ \h+ \i        { return '>'; }
^ \h+ \j        |
\j              { return '<'; }
\h+             { /* ignore spaces and tabs */ }
"---" {br}      { return 'S'; }
"..." {br}      { return 'E'; }
"-"             { return '-'; }
"?"             { return '?'; }
":"             { return ':'; }
","             { return ','; }
"["             { return '['; }
"]"             { return ']'; }
"{"             { return '{'; }
"}"             { return '}'; }
"'"             { clear(); start(APOS); }
\"              { clear(); start(QUOT); }
"|"  \d* {nl}   { clear(); parse_indent(1); mode = CLIP;  start(PRES); }
"|-" \d* {nl}   { clear(); parse_indent(2); mode = STRIP; start(PRES); }
"|+" \d* {nl}   { clear(); parse_indent(2); mode = KEEP;  start(PRES); }
">"  \d* {nl}   { clear(); parse_indent(1); mode = CLIP;  start(FOLD); }
">-" \d* {nl}   { clear(); parse_indent(2); mode = STRIP; start(FOLD); }
">+" \d* {nl}   { clear(); parse_indent(2); mode = KEEP;  start(FOLD); }
{tag}           { return chr(); }
{scalar}        { string = wstr(); return '$'; }

<APOS>{
'               { start(INITIAL); return '$'; }
''              { add(L'\''); }
}

<QUOT>{
\\ {lf}         { /* ignore \LF */ }
\"              { start(INITIAL); return '$'; }
\\ 0            { add(L'\0'); }
\\ a            { add(L'\a'); }
\\ b            { add(L'\b'); }
\\ t            { add(L'\t'); }
\\ n            { add(L'\n'); }
\\ v            { add(L'\v'); }
\\ f            { add(L'\f'); }
\\ r            { add(L'\r'); }
\\ e            { add(0x1b); }
\\ N            { add(0x85); }
\\ _            { add(0xa0); }
\\ L            { add(0x2028); }
\\ P            { add(0x2029); }
\\ x {h2}       { add(strtoul(text() + 2, NULL, 16)); }
\\ u {h4}       { add(strtoul(text() + 2, NULL, 16)); }
\\ U {h8}       { add(strtoul(text() + 2, NULL, 16)); }
\\ .            { add(wstr()[1]); }
}

<APOS,QUOT>{
^ \h+ \k?       { /* ignore nodent/undent */ }
\h* {lf}        { add(L' '); }
{bl}            { add(L'\n', newlines() - 1); }
.               { add(wchr()); }
}

<PRES>{
^ \h+ \i        { adjust_indent(); start(PBLK); }
}

<FOLD>{
^ \h+ \i        { adjust_indent(); sp = 0; nl = false; start(FBLK); }
}

<PBLK>{
{lf}            { add(L'\n'); }
{bl}            { add(L'\n', newlines()); }
^ \h* \j        |
\j              { chomp(); start(INITIAL); return '$'; }
^ \h+           { }
^ \h+ \k        { add_indent(); }
.               { add(wchr()); }
}

<FBLK>{
\h+ {lf}        { sp = size() - 1 - (*(matcher().end() - 2) == '\r'); }
{lf}            { sp = 1; }
{bl}            { add(L'\n', newlines() - 1); }
^ \h* \j        |
\j              { chomp(); start(INITIAL); return '$'; }
^ \h+           { add_newline(); }
^ \h+ \k        { sp = 0; nl = true; add_indent(); }
.               { add_space(); add(wchr()); }
}

%%

// YAML 1.2 value with YAML writer
class YAML {

 public:

  typedef std::wstring         Str; // YAML string (scalars)
  typedef std::vector<YAML>    Seq; // YAML sequence
  typedef std::pair<YAML,YAML> Duo;
  typedef std::vector<Duo>     Map; // YAML map

  YAML() { }

  void write(std::ostream& os, size_t indent, bool key) const
  {
    if (!str.empty())
    {
      write_string(os, str);
    }
    else if (!seq.empty())
    {
      for (YAML::Seq::const_iterator i = seq.begin(); i != seq.end(); ++i)
      {
        if (key)
        {
          os << "? ";
          key = false;
          ++indent;
        }
        else
        {
          write_indent(os, indent);
        }
        os << "- ";
        i->write(os, indent + 1, false);
      }
    }
    else if (!map.empty())
    {
      for (YAML::Map::const_iterator i = map.begin(); i != map.end(); ++i)
      {
        if (key)
        {
          os << "? ";
          key = false;
          ++indent;
        }
        else
        {
          write_indent(os, indent);
        }
        i->first.write(os, indent, !i->first.seq.empty() || !i->first.map.empty());
        if (!i->first.seq.empty() || !i->first.map.empty())
          write_indent(os, indent);
        os << ": ";
        i->second.write(os, indent + 1, false);
      }
    }
  }

  Str tag; // YAML tag, starts with '!'
  Str ref; // YAML anchor, starts with '&' or '*'
  Str str; // YAML string (scalar)
  Seq seq; // YAML sequence
  Map map; // YAML map

 protected:

  // write newline with indent
  static void write_indent(std::ostream& os, size_t indent)
  {
    os << '\n';
    while (indent-- > 0)
      os << "  ";
  }

  // write YAML quoted string
  static void write_string(std::ostream& os, const std::wstring& s)
  {
    os << '"';
    for (std::wstring::const_iterator i = s.begin(); i != s.end(); ++i)
    {
      switch (*i)
      {
        case '"' :
        case '\\': os << '\\' << static_cast<char>(*i); break;
        case '\0': os << "\\0"; break;
        case '\a': os << "\\a"; break;
        case '\b': os << "\\b"; break;
        case '\t': os << "\\t"; break;
        case '\n': os << "\\n"; break;
        case '\v': os << "\\v"; break;
        case '\f': os << "\\f"; break;
        case '\r': os << "\\r"; break;
        default  : if (*i >= '\x20' && *i <= '\x7f')
                   { // emit printable char
                     os << static_cast<char>(*i);
                   }
                   else if (*i < 0x20)
                   { // emit \xxx for control codes
                     os << "\\x" << std::internal << std::setw(2) << std::setfill('0') << std::hex << *i << std::dec;
                   }
                   else if (*i >= 0xD800 && *i < 0xE000)
                   { // UTF-16 surrogates
                     char buf[8];
                     int c = 0x010000 + ((*i - 0xD800) << 10);
                     c += *++i - 0xDC00;
                     buf[reflex::utf8(c, buf)] = '\0'; // convert to UTF-8 and make \0-terminated
                     os << buf;
                   }
                   else
                   { // else emit UTF-8
                     char buf[8];
                     buf[reflex::utf8(*i, buf)] = '\0'; // convert to UTF-8 and make \0-terminated
                     os << buf;
                   }
      }
    }
    os << '"';
  }
};

std::ostream& operator<<(std::ostream& os, const YAML& data)
{
  data.write(os, 0, false);
  return os;
}

// YAML 1.2 parser derived from the lexer
class YAMLParser : public Lexer {

 public:

  YAMLParser(FILE *fd = NULL) : Lexer(fd), token(lex()) { }

  // parse YAML documents
  void parse()
  {
    while (true)
    {
      YAML data;
      if (token == 'S')
        next();
      else if (token == 0 || token == 'E')
        break;
      parse(data);
      doc.push_back(data);
    }
  }

  // write YAML documents
  void write(std::ostream &os) const
  {
    for (YAML::Seq::const_iterator i = doc.begin(); i != doc.end(); ++i)
      os << "--- " << *i << '\n';
    os << "...\n";
  }

  YAML::Seq doc; // sequence of YAML documents parsed

 protected:

  // parse YAML data
  void parse(YAML& data)
  {
    if (token == '=' || token == ';')
      next();
    if (token == '!')
    {
      data.tag = string;
      next();
    }
    if (token == '&' || token == '*')
    {
      data.ref = string;
      next();
    }
    switch (token)
    {
      case '-': parse_seq(data); break;
      case '>': parse_ind(data); break;
      case '$': parse_str_or_map(data); break;
      case '[': parse_flow_seq(data); break;
      case '{': parse_flow_map(data); break;
      case '?': parse_key(data); break;
      case ':': parse_map(data); break;
      default:
#ifdef SHOW_TOKENS
                std::cout << "skipping " << (char)token << "\n";
#endif
                next();
                break;
    }
  }

  // parse "? key : val ..."
  void parse_key(YAML& data)
  {
    next();
    if (token == ';' || token == '=')
    {
      next();
      if (token == '>')
        parse_ind(data);
    }
    else
    {
      matcher().insert_stop(matcher().columno());
      parse(data);
      if (token == '<')
        next();
    }
    if (token == ':')
      parse_map(data);
  }

  // parse "- val ..."
  void parse_seq(YAML& data)
  {
    if (data.tag.empty())
      data.tag = L"!!seq";
    size_t level = 0;
    while (true)
    {
      if (token == '>')
      {
        next();
        ++level;
      }
      while (token == '<')
      {
        if (level == 0)
          return;
        next();
        if (level == 1)
          return;
        --level;
      }
      if (token != '-')
        break;
      next();
      YAML val;
      parse(val);
      data.seq.push_back(val);
    }
  }

  // parse indented value (string, nested sequence, or nested map)
  void parse_ind(YAML& data)
  {
    next();
    if (token == '-')
    {
      parse_seq(data);
    }
    else
    {
      bool sp = true;
      size_t level = 0;
      while (token == '$')
      {
        if (data.str.empty())
          data.str = string;
        else if (sp)
          data.str.append(L" ").append(string);
        else
          data.str.append(string);
        sp = true;
        next();
        if (token == ';')
        {
          next();
        }
        else if (token == '=')
        {
          data.str.append(newlines() - 1, L'\n');
          sp = false;
          next();
        }
        else
        {
          break;
        }
        if (token == '>')
        {
          ++level;
          data.str.push_back(L'\n');
          sp = false;
          next();
        }
        while (token == '<')
        {
          if (level == 0)
            break;
          next();
          --level;
        }
      }
    }
    if (token == ':')
      parse_map(data);
    if (token == '<')
      next();
  }

  // parse string
  void parse_str(YAML& data)
  {
    if (data.tag.empty())
      data.tag = L"!!str"; 
    data.str = string;
    next();
    if (token == ';')
      next();
  }

  // parse string of "key: val ..."
  void parse_str_or_map(YAML& data)
  {
    if (data.tag.empty())
      data.tag = L"!!str"; 
    data.str = string;
    next();
    if (token == ':')
      parse_map(data);
    else if (token == ';' || token == '=')
      next();
  }

  // key is given in data, now parse ": val ..."
  void parse_map(YAML& data)
  {
    next();
    YAML val1;
    size_t level = 0;
    if (token == ';' || token == '=')
    {
      next();
      if (token != 0 && token != 'S' && token != 'E' && token != '$' && token != '?' && token != ':' && token != '<')
        parse(val1);
    }
    else if (token != 0 && token != 'S' && token != 'E' && token != '?' && token != ':' && token != '<')
    {
      parse(val1);
    }
    YAML::Duo duo(data, val1);
    data.tag = L"!!map"; 
    data.str.clear();
    data.seq.clear();
    data.map.clear();
    data.map.push_back(duo);
    while (token != 0 && token != 'S' && token != 'E' && token != '<')
    {
      YAML key, val;
      if (token == ';' || token == '=')
        next();
      if (token == '>')
      {
        next();
        ++level;
      }
      if (token == '$')
      {
        parse_str(key);
      }
      else if (token == '?')
      {
        next();
        if (token == ';' || token == '=')
        {
          next();
          if (token == '-')
            parse_seq(key);
          else if (token == '>')
            parse_ind(key);
        }
        else
        {
          matcher().insert_stop(matcher().columno());
          parse(key);
          if (token == '<')
            next();
        }
      }
      if (token == ':')
      {
        next();
        if (token == '&' || token == '*')
        {
          data.ref = string;
          next();
        }
        if (token == ';' || token == '=')
        {
          next();
          if (token == '-')
            parse_seq(val);
          else if (token == '>')
            parse_ind(val);
        }
        else
        {
          matcher().insert_stop(matcher().columno());
          parse(val);
          if (token == '<')
          {
            next();
            while (token == '<' && level > 0)
            {
              next();
              --level;
            }
          }
        }
      }
      else
      {
        break;
      }
      data.map.push_back(YAML::Duo(key, val));
    }
  }

  // parse "[ val, ... ]"
  void parse_flow_seq(YAML& data)
  {
    if (data.tag.empty())
      data.tag = L"!!seq";
    size_t level = 0;
    next();
    while (token != 0 && token != 'S' && token != 'E' && token != ']')
    {
      YAML val;
      if (token == ';' || token == '=')
        next();
      if (token == '>')
      {
        ++level;
        next();
      }
      else
      {
        while (token == '<')
        {
          if (level > 0)
            --level;
          next();
        }
      }
      parse(val);
      data.seq.push_back(val);
      if (token == ';' || token == '=')
        next();
      if (token == '>')
      {
        ++level;
        next();
      }
      else
      {
        while (token == '<')
        {
          if (level > 0)
            --level;
          next();
        }
      }
      if (token == ',')
        next();
      if (token == ';' || token == '=')
        next();
      if (token == '>')
      {
        ++level;
        next();
      }
      else
      {
        while (token == '<')
        {
          if (level > 0)
            --level;
          next();
        }
      }
    }
    if (token == ']')
      next();
    if (token == ';' || token == '=')
      next();
    while (token == '<' && level-- > 0)
      next();
  }

  // parse "{ key:val, ... }"
  void parse_flow_map(YAML& data)
  {
    if (data.tag.empty())
      data.tag = L"!!map";
    size_t level = 0;
    next();
    while (token != 0 && token != 'S' && token != 'E' && token != '}')
    {
      YAML key, val;
      if (token == ';' || token == '=')
        next();
      if (token == '>')
      {
        ++level;
        next();
      }
      else
      {
        while (token == '<')
        {
          if (level > 0)
            --level;
          next();
        }
      }
      if (token == '$')
        parse_str(key);
      else if (token == '[')
        parse_flow_seq(key);
      else if (token == '{')
        parse_flow_map(key);
      if (token == ':')
      {
        next();
        if (token != ';' && token != '=' && token != ',')
          parse(val);
      }
      data.map.push_back(YAML::Duo(key, val));
      if (token == ';' || token == '=')
        next();
      if (token == '>')
      {
        ++level;
        next();
      }
      else
      {
        while (token == '<')
        {
          if (level > 0)
            --level;
          next();
        }
      }
      if (token == ',')
        next();
      if (token == ';' || token == '=')
        next();
      if (token == '>')
      {
        ++level;
        next();
      }
      else if (token == '<')
      {
        if (level > 0)
          --level;
        next();
      }
    }
    if (token == '}')
      next();
    if (token == ';' || token == '=')
      next();
    while (token == '<' && level-- > 0)
      next();
  }

  // get next token, zero when EOF
  int next()
  {
#ifdef SHOW_TOKENS // produce token sequence for debugging
    std::cout << "token " << (char)token << '\n';
    if (token == '$')
      std::wcout << L">>>" << string << L"<<<\n";
#endif
    if (token != 0)
      token = lex();
    return token;
  }

  int token;
};

std::ostream& operator<<(std::ostream& os, const YAMLParser& parser)
{
  parser.write(os);
  return os;
}

// The main program parses YAML from a file or from stdin and writes it
int main(int argc, char **argv)
{
  FILE *fd = stdin;
  // open file if a file name is given on the command line
  if (argc > 1 && (fd = fopen(argv[1], "r")) == NULL)
    exit(EXIT_FAILURE);
  YAMLParser yaml(fd);
  yaml.parse();
  std::cout << yaml;
}
