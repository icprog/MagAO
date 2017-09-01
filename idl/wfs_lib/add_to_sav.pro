
pro add_to_sav, filename, params

    save, /variables, FILE = filename+'_params.sav'
    ;restore,filename
    ;save, /variables, FILE=filename
end
    
