#include <stdio.h>
#include <string.h>
#include <elf.h>
#include <dlfcn.h>

extern void bad()
{
	printf("bad() was called!\n");
}

Elf64_Shdr *get_section_by_type(Elf64_Ehdr *module, Elf64_Word type)
{
	Elf64_Shdr *section_header = (Elf64_Shdr*)((char*)module + module->e_shoff);
	for (size_t i = 0; i < module->e_shnum; i++)
	{
		fprintf(stderr, "Section type: %d\n", section_header[i].sh_type);
		if (section_header[i].sh_type == type)
			return section_header;
	}
	return NULL;
}

void print_program_header(Elf64_Phdr *program_header)
{
	printf("\tp_type=%d\n", program_header->p_type);
	printf("\tp_flags=0x%x\n", program_header->p_flags);
	printf("\tp_offset=0x%lx\n", program_header->p_offset);
	printf("\tp_vaddr=0x%lx\n", program_header->p_vaddr);
	printf("\tp_paddr=0x%lx\n", program_header->p_paddr);
	printf("\tp_filesz=%ld\n", program_header->p_filesz);
	printf("\tp_memsz=%ld\n", program_header->p_memsz);
	printf("\tp_align=%ld\n", program_header->p_align);
}

void print_program_headers(Elf64_Ehdr *module)
{
	Elf64_Phdr *program_header = (Elf64_Phdr*)((char*)module + module->e_phoff);
	for (size_t i = 0; i < module->e_phnum; i++)
	{
		printf("---- Program header %ld ----\n", i);
		print_program_header(&program_header[i]);
	}
}

Elf64_Phdr *get_program_header_by_type(Elf64_Ehdr *module, Elf64_Word type)
{
	Elf64_Phdr *program_header = (Elf64_Phdr*)((char*)module + module->e_phoff);
	for (size_t i = 0; i < module->e_phnum; i++)
	{
		if (program_header[i].p_type == type)
			return &program_header[i];
	}
	return NULL;
}

void print_dynamic_entries(Elf64_Ehdr *module)
{
	Elf64_Phdr *dynamic_header = get_program_header_by_type(module, PT_DYNAMIC);
	if (!dynamic_header)
	{
		perror("Could not find dynamic program header\n");
		return;
	}
	Elf64_Dyn *current = (Elf64_Dyn*)((char*)module+dynamic_header->p_vaddr);
	size_t cur_size = 0;
	printf("---- Dynamic Entries ----\n");
	while (cur_size < dynamic_header->p_memsz)
	{
		printf("d_tag=%016lx,\td_val=%016lx\n", current->d_tag, current->d_un.d_val);
		current++;
		cur_size += sizeof(Elf64_Dyn);
	}
	printf("---- End of Dynamic Entries ----\n");
}

Elf64_Dyn *get_dynamic_entry(Elf64_Ehdr *module, Elf64_Sword entry_type)
{
	Elf64_Phdr *dynamic_header = get_program_header_by_type(module, PT_DYNAMIC);
	if (!dynamic_header)
	{
		perror("Could not find dynamic program header\n");
		return NULL;
	}
	Elf64_Dyn *current = (Elf64_Dyn*)((char*)module+dynamic_header->p_vaddr);
	size_t cur_size = 0;
	while (cur_size < dynamic_header->p_memsz)
	{
		if (current->d_tag == entry_type)
			return current;
		current++;
		cur_size += sizeof(Elf64_Dyn);
	}
	return NULL;
}

void print_symbol(Elf64_Sym *symbol, char *str_tbl)
{
	printf("---- Symbol: '%s' ----\n", &str_tbl[symbol->st_name]);
	printf("\tsh_name='%s'\n", &str_tbl[symbol->st_name]);
	printf("\tst_info=0x%x\n", symbol->st_info);
	printf("\tst_other=0x%x\n", symbol->st_other);
	printf("\tst_shndx=%d\n", symbol->st_shndx);
	printf("\tst_value=0x%lx\n", symbol->st_value);
	printf("\tst_size=%ld\n", symbol->st_size);
}

void print_symbol_table(Elf64_Ehdr *module)
{
	Elf64_Dyn *sym_tbl_dyn = get_dynamic_entry(module, DT_SYMTAB); // Symbol table dynamic entry.
	Elf64_Dyn *str_tbl_dyn = get_dynamic_entry(module, DT_STRTAB); // String table dynamic entry (for symbol names).
	if (!sym_tbl_dyn || !str_tbl_dyn)
	{
		perror("Could not find dynamic entry of type DT_SYMTAB or DT_STRTAB.\n");
		return;
	}

	Elf64_Sym *sym_tbl = (Elf64_Sym*)(sym_tbl_dyn->d_un.d_ptr);
	char *str_tbl = (char*)(str_tbl_dyn->d_un.d_ptr);
	
	size_t index = 0;
	while (index < 12)
	{
		print_symbol(sym_tbl, str_tbl);
		sym_tbl++;
		index++;
	}
}
/*
unsigned long int get_static_func_by_name(Elf64_Ehdr *module, const char *func_name)
{
	Elf64_Dyn *sym_dyn = get_dynamic_entry(module, DT_SYMTAB); // Get dynamic entry for symbol table.
	Elf64_Dyn *str_dyn = get_dynamic_entry(module, DT_STRTAB); // Get dynamic entry for string table.
	if (!sym_dyn || !str_dyn)
	{
		perror("Could not find dynamic entry for either symbol table or string table.\n");
		return 0;
	}
	Elf64_Sym *sym_tbl = (Elf64_Sym*)(sym_dyn->d_un.d_ptr); // Get symbol table.
	char *str_tbl = (char*)(str_dyn->d_un.d_ptr); // Get string table.

	size_t cur_size = 0;
	while (cur_size < 0)
	{
		if (!strcmp(func_name, (char*)str_tbl + sym_tbl->st_name))
			return (unsigned long int)sym_tbl->st_value;
		cur_size += 0;
		sym_hdr++;
	}
	return 0;
}
*/

void __attribute__ ((constructor)) init(void)
{
	printf("libbad was loaded!\n");
	Elf64_Ehdr **pmodule = (Elf64_Ehdr**)dlopen(NULL, RTLD_LAZY);
	if (!pmodule)
	{
		perror("Unable to load module!\n");
		return;
	}
	//unsigned long int func_addr = get_static_func_by_name(*pmodule, "good");
	//printf("Found good() at address %016lx\n", func_addr);
	//print_program_headers(*pmodule);
	//print_dynamic_entries(*pmodule);
	//print_symbol_table(*pmodule);
	Elf64_Dyn *pltrel = get_dynamic_entry(*pmodule, DT_PLTGOT);
	printf("%08lx\n", pltrel->d_un.d_ptr);
	printf("%08x %08x %08x\n", *(int*)pltrel->d_un.d_ptr, *(int*)pltrel->d_un.d_ptr+4, *(int*)pltrel->d_un.d_ptr+8);
}

