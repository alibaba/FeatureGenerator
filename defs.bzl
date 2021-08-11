def gen_cpp_code(name, elements_list, template_header, template, template_tail,
                 element_per_file = 20, suffix=".cpp"):
    bases = []
    base = 1

    for i in range(len(elements_list)):
        base = len(elements_list[i]) * base

    base_tmp = base
    for i in range(len(elements_list)):
        base_tmp = base_tmp // len(elements_list[i])
        bases.append(base_tmp)

    files = []
    current = 0
    count = 0
    current_str = template_header
    for i in range(base):
        replace_elements_list = []
        num = i
        for j in range(len(bases)):
            replace_elements_list.append(elements_list[j][num // bases[j]])
            num %= bases[j]
        # for all permutations here

        if type(replace_elements_list[0]) == "tuple":
            replace_elements_list = replace_elements_list[0]
        else:
            replace_elements_list = tuple(replace_elements_list)
        current_str += template.format(*replace_elements_list)
        current += 1
        if current == element_per_file or i == base - 1:
            cpp_name = name + "_" + str(count)
            count += 1
            file_name = cpp_name + suffix
            content = current_str + template_tail
            native.genrule(
                name = cpp_name,
                srcs = [],
                outs = [file_name],
                cmd = "cat > $@  << 'EOF'\n" + content + "EOF",
            )
            current = 0
            current_str = template_header
            files.append(cpp_name)

    native.filegroup(
        name = name,
        srcs = files
    )
