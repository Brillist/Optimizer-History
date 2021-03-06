#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////

GOP_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Data-set (abstract).

   A full specification of an optimization problem.

   \ingroup gop
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class DataSet : public utl::Object
{
    UTL_CLASS_DECL_ABC(DataSet, utl::Object);
    UTL_CLASS_DEFID;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

GOP_NS_END;
