#include "DiagnSet.h"

DiagnSetPtr  operator| (DiagnSetPtr uno, DiagnSetPtr due)
{
    DiagnSetPtr  unione (new  DiagnSet());
    //unione->insert(unione->end(), uno->begin(), uno->end());
    //for (DiagnSet::iterator it = due->begin(); it != due->end();  it++)
    //{
    //   try{
    //        unione->Add(*it );
    //    } catch (DiagnSetException &e) {continue;}
    //}
    unione->resize(uno->size()+due->size() );
    std::vector<DiagnVarPtr>::iterator it;
    std::sort(uno->begin(), uno->end(), less<DiagnVarPtr>());
    std::sort(due->begin(), due->end(), less<DiagnVarPtr>());
    it=std::set_union (uno->begin(), uno->end(), due->begin(), due->end(), unione->begin(), less<DiagnVarPtr>());
    unione->resize(it - unione->begin());
    return unione;
}

DiagnSetPtr  operator& (DiagnSetPtr uno, DiagnSetPtr due)
{
    DiagnSetPtr  intersezione (new  DiagnSet());
    intersezione->resize(uno->size()+due->size() );
    std::vector<DiagnVarPtr>::iterator it;
    std::sort(uno->begin(), uno->end(), less<DiagnVarPtr>());
    std::sort(due->begin(), due->end(), less<DiagnVarPtr>());
    it=std::set_intersection (uno->begin(), uno->end(), due->begin(), due->end(), intersezione->begin(), less<DiagnVarPtr>());
    intersezione->resize(it - intersezione->begin());
    return intersezione;
}

void DiagnSet::Add (DiagnVarPtr varptr)
{ 
    // se c'e' di gia' tira un'eccezione
    try {
        Find(MatchDiagnVar(varptr->Family(), varptr->Index()));
    } catch (DiagnSetException &e) {
        this->push_back(varptr);
        return;
    }
    std::ostringstream os;
    os << varptr->Name() << "  already inserted in the diagnvar database " << std::ends;
    throw DiagnSetException(os.str(), NO_ERROR, __FILE__,__LINE__);
    return;
}

DiagnSetPtr DiagnSet::Filter(const DiagnWhich& filter_var)
{
    DiagnSetPtr  filtered (new  DiagnSet());
    std::copy_if(this->begin(), this->end(), std::back_inserter(*filtered), filter_var);
    return filtered;
}

DiagnVarPtr DiagnSet::Find(const MatchDiagnVar& matchvar )
{
    std::vector<DiagnVarPtr>::iterator ret;
    ret = std::find_if(this->begin(), this->end(), matchvar);
    if (ret != this->end()) return *ret;
    else {
        std::ostringstream os;
        os << matchvar << "  not found" << std::ends;
        throw DiagnSetException(os.str(), NO_ERROR, __FILE__,__LINE__);
    }
}

vector<double> DiagnSet::GetValues()
{
    std::vector<double> vvv(this->size());
    int i=0;
    for (DiagnSet::iterator it = this->begin(); it != this->end();  it++, i++){
        vvv[i] =  (*it)->Value();
    }
    return vvv;
}

