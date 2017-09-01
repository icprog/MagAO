; $Id: gen_dsp_nios_structure.pro,v 1.2 2007/02/07 18:06:08 marco Exp $

function gen_dsp_nios_structure

uint16 = 0U

dsp_nios_structure = $
{ $
	who_ami         : uint16, $; // identificazione della board
	software_release: uint16, $; // lettura del PIO con la versione del software
	logic_release   : uint16, $; // lettura del PIO con la versione della logica
	dummy_word1     : uint16  $; // dummy word per allineare a 32 bit
}

return, dsp_nios_structure
end
