"""
add missing type_annotations to VEX code.
usage:
new_lines = fix_types(lines)
"""

lookup = {
    'active':'i',
    'stopped':'i',
    'dead':'i',
    'id':'i',
    'ptnum':'i',
    'primnum':'i',
    'Cd':'v',
    'N':'v',
    'P':'v',
    'v':'v',
    'vel':'v',
    'rest':'v',
    'w':'v',
    'name':'s',
    'orient':'p'
}

def fix_type_annotations(s:str) -> str:
    eq = s.split("=")
    if len(eq)!=2:
        return s
    else:
        v = eq[0].split("@")

        typedef = "f"
        if v[0]=="":
            value = lookup.get(v[1])
            if value:
                typedef = value

        v[0]=typedef
        new_string = "@".join(v);

        eq[0] = new_string
        newline = "=".join(eq)

        return newline

def fix_types(text:str ) -> str:
    lines = text.splitlines()
    for i, line in enumerate(lines):
        if "@" in line:
            fixed_line =  fix_type_annotations(line)
            lines[i] = fixed_line
    return "\n".join(lines)


with open("vex.txt","r") as vex:
    lines = vex.read()
    new_lines = fix_types(lines)
    print(lines)
    print("------------------------")
    print(new_lines)



