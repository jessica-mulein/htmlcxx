#include "htmlcxx2_html.hpp"

namespace htmlcxx2 {
namespace HTML {

//
// Node
//

size_t Node::parseAttributes()
{
    if (!isTag())
        return 0;

    const char *end;
    const char *ptr = text_.c_str();
    if ((ptr = strchr(ptr, '<')) == 0)
        return 0;
    ++ptr;

    // Skip initial blankspace
    while (::isspace((unsigned char)*ptr))
        ++ptr;

    // Skip tagname
    if (!::isalpha((unsigned char)*ptr))
        return 0;
    while (!::isspace((unsigned char)*ptr) && *ptr != '>')
        ++ptr;

    // Skip blankspace after tagname
    while (::isspace((unsigned char)*ptr))
        ++ptr;

    while (*ptr && *ptr != '>')
    {
        std::string key, val;

        // skip unrecognized
        while (*ptr && !::isalnum((unsigned char)*ptr) && !::isspace((unsigned char)*ptr))
            ++ptr;

        // skip blankspace
        while (::isspace((unsigned char)*ptr))
            ++ptr;

        end = ptr;
        while (::isalnum((unsigned char)*end) || *end == '-')
            ++end;
        key.assign(end - ptr, '\0');
        std::transform(ptr, end, key.begin(), ::tolower);
        ptr = end;

        // skip blankspace
        while (::isspace((unsigned char)*ptr))
            ++ptr;

        if (*ptr == '=')
        {
            ++ptr;
            while (::isspace((unsigned char)*ptr))
                ++ptr;
            if (*ptr == '"' || *ptr == '\'')
            {
                char quote = *ptr;
                end = strchr(ptr + 1, quote);
                if (end == 0)
                {
                    const char *end1, *end2;
                    end1 = strchr(ptr + 1, ' ');
                    end2 = strchr(ptr + 1, '>');
                    end = end1 && (end1 < end2) ? end1 : end2;
                    if (end == 0)
                        return attributeKeys_.size();
                }
                const char *begin = ptr + 1;
                while (::isspace((unsigned char)*begin) && begin < end)
                    ++begin;
                const char *trimmed_end = end - 1;
                while (::isspace((unsigned char)*trimmed_end) && trimmed_end >= begin)
                    --trimmed_end;
                val.assign(begin, trimmed_end + 1);
                ptr = end + 1;
            }
            else
            {
                end = ptr;
                while (*end && !::isspace((unsigned char)*end) && *end != '>')
                    end++;
                val.assign(ptr, end);
                ptr = end;
            }
            addAttribute(key, val);
        }
        else if (!key.empty())
            addAttribute(key);
    }
}

//
// ParserDom
//

void ParserDom::onFoundTag(Node &node, bool isClosingTag)
{
    if (!isClosingTag)
    {
        //append to current tree node
        Tree::iterator nextIt;
        nextIt = tree_.append_child(currIt_, node);
        currIt_ = nextIt;
    }
    else
    {
        //Look if there is a pending open tag with that same name upwards
        //If currIt_ tag isn't matching tag, maybe a some of its parents
        // matches
        std::vector<Tree::iterator> path;
        Tree::iterator i = currIt_;
        bool foundOpenTag = false;
        while (i != tree_.begin())
        {
            assert(i->isTag());
            assert(i->tagName().length());

            bool equal;
            const char *open = i->tagName().c_str();
            const char *close = node.tagName().c_str();
            equal = detail::icompare(open, close) == 0;

            if (equal)
            {
                //Closing tag closes this tag
                //Set length to full range between the opening tag and
                //closing tag
                i->setLength(node.offset() + node.length() - i->offset());
                i->setClosingText(node.text());
                // TODO: set node's content text

                currIt_ = tree_.parent(i);
                foundOpenTag = true;
                break;
            }
            else
                path.push_back(i);
            i = tree_.parent(i);
        }

        if (foundOpenTag)
        {
            //If match was upper in the tree, so we need to invalidate child
            //nodes that were waiting for a close
            for (size_t j = 0; j < path.size(); ++j)
                tree_.flatten(path[j]);
        }
        else
        {
            // Treat as comment
            node.setKindComment();
            tree_.append_child(currIt_, node);
        }
    }
}

//
// Utils
//

namespace {

    struct
    {
        const char *str;
        unsigned char chr;
    } entities[] =
    {
        /* 00 */
        { "quot", 34 },
        { "amp", 38 },
        { "lt", 60 },
        { "gt", 62 },
        { "nbsp", ' ' },
        { "iexcl", 161 },
        { "cent", 162 },
        { "pound", 163 },
        { "curren", 164 },
        { "yen", 165 },
        /* 10 */
        { "brvbar", 166 },
        { "sect", 167 },
        { "uml", 168 },
        { "copy", 169 },
        { "ordf", 170 },
        { "laquo", 171 },
        { "not", 172 },
        { "shy", 173 },
        { "reg", 174 },
        { "macr", 175 },
        /* 20 */
        { "deg", 176 },
        { "plusmn", 177 },
        { "sup2", 178 },
        { "sup3", 179 },
        { "acute", 180 },
        { "micro", 181 },
        { "para", 182 },
        { "middot", 183 },
        { "cedil", 184 },
        { "sup1", 185 },
        /* 30 */
        { "ordm", 186 },
        { "raquo", 187 },
        { "frac14", 188 },
        { "frac12", 189 },
        { "frac34", 190 },
        { "iquest", 191 },
        { "Agrave", 192 },
        { "Aacute", 193 },
        { "Acirc", 194 },
        { "Atilde", 195 },
        /* 40 */
        { "Auml", 196 },
        { "ring", 197 },
        { "AElig", 198 },
        { "Ccedil", 199 },
        { "Egrave", 200 },
        { "Eacute", 201 },
        { "Ecirc", 202 },
        { "Euml", 203 },
        { "Igrave", 204 },
        { "Iacute", 205 },
        /* 50 */
        { "Icirc", 206 },
        { "Iuml", 207 },
        { "ETH", 208 },
        { "Ntilde", 209 },
        { "Ograve", 210 },
        { "Oacute", 211 },
        { "Ocirc", 212 },
        { "Otilde", 213 },
        { "Ouml", 214 },
        { "times", 215 },
        /* 60 */
        { "Oslash", 216 },
        { "Ugrave", 217 },
        { "Uacute", 218 },
        { "Ucirc", 219 },
        { "Uuml", 220 },
        { "Yacute", 221 },
        { "THORN", 222 },
        { "szlig", 223 },
        { "agrave", 224 },
        { "aacute", 225 },
        /* 70 */
        { "acirc", 226 },
        { "atilde", 227 },
        { "auml", 228 },
        { "aring", 229 },
        { "aelig", 230 },
        { "ccedil", 231 },
        { "egrave", 232 },
        { "eacute", 233 },
        { "ecirc", 234 },
        { "euml", 235 },
        /* 80 */
        { "igrave", 236 },
        { "iacute", 237 },
        { "icirc", 238 },
        { "iuml", 239 },
        { "ieth", 240 },
        { "ntilde", 241 },
        { "ograve", 242 },
        { "oacute", 243 },
        { "ocirc", 244 },
        { "otilde", 245 },
        /* 90 */
        { "ouml", 246 },
        { "divide", 247 },
        { "oslash", 248 },
        { "ugrave", 249 },
        { "uacute", 250 },
        { "ucirc", 251 },
        { "uuml", 252 },
        { "yacute", 253 },
        { "thorn", 254 },
        { "yuml", 255 },
        /* 100 */
        { NULL, 0 },
    };

}

std::string decodeEntities(const std::string &str)
{
    size_t count = 0;
    const char *ptr = str.c_str();
    const char *end;

    std::string ret(str);
    std::string entity;

    ptr = strchr(ptr, '&');
    if (ptr == NULL)
        return ret;

    count += ptr - str.c_str();

    while (*ptr)
    {
        if (*ptr == '&' && ((end = strchr(ptr, ';')) != NULL))
        {
            entity.assign(ptr + 1, end);
            if (!entity.empty() && entity[0] == '#')
            {
                entity.erase(0, 1);
                int chr = atoi(entity.c_str());
                if (chr > 0 && chr <= UCHAR_MAX)
                {
                    ret[count++] = (unsigned char)chr;
                }
                ptr = end + 1;
            }
            else
            {
                bool found = false;
                for (int i = 0; entities[i].str != NULL; i++)
                {
                    if (entity == entities[i].str)
                    {
                        found = true;
                        ret[count++] = entities[i].chr;
                        ptr = end + 1;
                        break;
                    }
                }

                if (!found)
                {
                    ret[count++] = *ptr++;
                }
            }
        }
        else
        {
            ret[count++] = *ptr++;
        }
    }

    ret.erase(count);
    return ret;
}

#ifdef _DEBUG
std::ostream& operator<<(std::ostream &stream, const Tree &tr)
{
    Tree::pre_order_iterator it = tr.begin();
    Tree::pre_order_iterator end = tr.end();

    int rootdepth = tr.depth(it);
    stream << "-----" << std::endl;

    size_t n = 0;
    while (it != end)
    {

        int cur_depth = tr.depth(it);
        for (int i = 0; i < cur_depth - rootdepth; ++i)
            stream << "  ";
        stream << n << "@";
        stream << "[" << it->offset() << ";";
        stream << it->offset() + it->length() << ") ";
        stream << ((*it).isTag() ? (*it).tagName() : (*it).text()) << std::endl;
        ++it, ++n;
    }
    stream << "-----" << std::endl;
    return stream;
}
#endif

} }