//+==================================================================================================================
//
// file :               MultiAttribute.cpp
//
// description :        C++ source code for the MultiAttribute class. This class is used to manage attribute.
//						A Tango Device object instance has one MultiAttribute object which is an aggregate of
//						Attribute or WAttribute objects
//
// project :            TANGO
//
// author(s) :          E.Taurel
//
// Copyright (C) :      2004,2005,2006,2007,2008,2009,2010,2011,2012,2013,2014,2015
//						European Synchrotron Radiation Facility
//                      BP 220, Grenoble 38043
//                      FRANCE
//
// This file is part of Tango.
//
// Tango is free software: you can redistribute it and/or modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Tango is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
// of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License along with Tango.
// If not, see <http://www.gnu.org/licenses/>.
//
// $Revision$
//
//-==================================================================================================================

#if HAVE_CONFIG_H
#include <ac_config.h>
#endif

#include <tango.h>
#include <eventsupplier.h>
#include <multiattribute.h>
#include <classattribute.h>

#include <functional>
#include <algorithm>

namespace Tango
{

//
// Define the optional attribute name and default value
//

static OptAttrProp Tango_OptAttrProp[] = {
	{"label",LabelNotSpec},
	{"description",DescNotSpec},
	{"unit",UnitNotSpec},
	{"standard_unit",StdUnitNotSpec},
	{"display_unit",DispUnitNotSpec},
	{"format",FormatNotSpec_FL},
	{"min_value",AlrmValueNotSpec},
	{"max_value",AlrmValueNotSpec},
	{"min_alarm",AlrmValueNotSpec},
	{"max_alarm",AlrmValueNotSpec},
	{"writable_attr_name",AssocWritNotSpec},
	{"min_warning",AlrmValueNotSpec},
	{"max_warning",AlrmValueNotSpec},
	{"delta_t","0"},
	{"delta_val",AlrmValueNotSpec},
	{"root_attr_name",AssocWritNotSpec}
};


//+------------------------------------------------------------------------------------------------------------------
//
// method :
//		MultiAttribute::MultiAttribute
//
// description :
//		Constructor for the MultiAttribute class from the device device name and a pointer to the DeviceClass object
//
// argument :
//		in :
//			- dev_name : The device name
//			- dev_class_ptr : Pointer to the DeviceClass object
//			- dev : Device pointer
//
//------------------------------------------------------------------------------------------------------------------

MultiAttribute::MultiAttribute(string &dev_name,DeviceClass *dev_class_ptr,DeviceImpl *dev)
:ext(new MultiAttribute::MultiAttributeExt)
{
	long i;
	cout4 << "Entering MultiAttribute class constructor for device " << dev_name << endl;

//
// Retrieve attr name list
//

	vector<Attr *> &tmp_attr_list = dev_class_ptr->get_class_attr()->get_attr_list();
	long nb_attr = tmp_attr_list.size();

//
// Get device attribute properties
// No need to implement a retry here (in case of db server restart) because the db reconnection
// is forced by the get_property call executed during xxxClass construction before we reach this code.
//

    cout4 << nb_attr << " attribute(s)" << endl;

	if (nb_attr != 0)
	{
		Tango::Util *tg = Tango::Util::instance();
		Tango::DbData db_list;

		if (tg->_UseDb == true)
		{
			for (i = 0;i < nb_attr;i++)
				db_list.push_back(DbDatum(tmp_attr_list[i]->get_name()));

//
// On some small and old computers, this request could take time if at the same time some other processes also access
// the device attribute properties table. This has been experimented at ESRF. Increase timeout to cover this case
//


            int old_db_timeout = 0;
            if (Util::_FileDb == false)
                old_db_timeout = tg->get_database()->get_timeout_millis();
			try
			{
			    if (old_db_timeout != 0)
                    tg->get_database()->set_timeout_millis(6000);
				tg->get_database()->get_device_attribute_property(dev_name,db_list,tg->get_db_cache());
				if (old_db_timeout != 0)
                    tg->get_database()->set_timeout_millis(old_db_timeout);
			}
			catch (Tango::DevFailed &)
			{
			    cout4 << "Exception while accessing database" << endl;

				tg->get_database()->set_timeout_millis(old_db_timeout);
				TangoSys_OMemStream o;
				o << "Can't get device attribute properties for device " << dev_name << ends;

				Except::throw_exception((const char *)API_DatabaseAccess,
				                	o.str(),
				                	(const char *)"MultiAttribute::MultiAttribute");
			}
		}

//
// Build property list for each attribute
//

		long ind = 0;
		vector<string> rem_att_name;
		vector<string> rem_att_cl_name;
		int sub = 0;

		for (i = 0;i < nb_attr;i++)
		{

//
// Get attribute class properties
//

			Attr &attr = dev_class_ptr->get_class_attr()->get_attr(tmp_attr_list[i]->get_name());
			vector<AttrProperty> &class_prop = attr.get_class_properties();
			vector<AttrProperty> &def_user_prop = attr.get_user_default_properties();

//
// If the attribute has some properties defined at device level, build a vector of these properties
//

			vector<AttrProperty> dev_prop;

			if (tg->_UseDb == true)
			{
				long nb_prop = 0;
				db_list[ind] >> nb_prop;
				ind++;

				for (long j = 0;j < nb_prop;j++)
				{
					if (db_list[ind].size() > 1)
					{
						string tmp(db_list[ind].value_string[0]);
						long nb = db_list[ind].size();
						for (int k = 1;k < nb;k++)
						{
							tmp = tmp + ",";
							tmp = tmp + db_list[ind].value_string[k];
						}
						dev_prop.push_back(AttrProperty(db_list[ind].name,tmp));
					}
					else
						dev_prop.push_back(AttrProperty(db_list[ind].name,db_list[ind].value_string[0]));
					ind++;
				}
			}

//
// Concatenate these two attribute properties levels
//

			vector<AttrProperty> prop_list;
			bool fwd_ok = true;
			concat(dev_prop,class_prop,prop_list);

			if (attr.is_fwd() == false)
			{
				add_user_default(prop_list,def_user_prop);
				add_default(prop_list,dev_name,attr.get_name(),attr.get_type());
			}
			else
			{
				FwdAttr &fwdattr = static_cast<FwdAttr &>(attr);
				fwd_ok = fwdattr.validate_fwd_att(prop_list,dev_name);
				dev->set_with_fwd_att(true);
				if (fwd_ok == true)
				{
					try
					{
						fwdattr.get_root_conf(dev_name,dev);
						fwdattr.remove_useless_prop(prop_list,dev_name,this);
						add_user_default(prop_list,def_user_prop);
						add_default(prop_list,dev_name,attr.get_name(),attr.get_type());
					}
					catch (Tango::DevFailed &e)
					{
						fwd_ok = false;

						add_user_default(prop_list,def_user_prop);
						add_default(prop_list,dev_name,attr.get_name(),attr.get_type());

						vector<DeviceImpl::FwdWrongConf> &fwd_wrong_conf = dev->get_fwd_att_wrong_conf();
						DeviceImpl::FwdWrongConf fwc;
						fwc.att_name = attr.get_name();
						fwc.full_root_att_name = fwdattr.get_full_root_att();
						fwc.fae = fwdattr.get_err_kind();
						fwd_wrong_conf.push_back(fwc);

						if (fwc.fae == FWD_ROOT_DEV_NOT_STARTED)
						{
							rem_att_name.push_back(fwdattr.get_name());
							rem_att_cl_name.push_back(fwdattr.get_cl_name());
						}
					}
				}
				else
				{
					vector<DeviceImpl::FwdWrongConf> &fwd_wrong_conf = dev->get_fwd_att_wrong_conf();
					DeviceImpl::FwdWrongConf fwc;
					fwc.att_name = attr.get_name();
					fwc.full_root_att_name = fwdattr.get_full_root_att();
					fwc.fae = fwdattr.get_err_kind();
					fwd_wrong_conf.push_back(fwc);
				}
			}

//
// Create an Attribute instance
//

			if (fwd_ok == true)
			{
				if ((attr.get_writable() == Tango::WRITE) ||
					(attr.get_writable() == Tango::READ_WRITE))
				{
					if (attr.is_fwd() == true)
					{
						Attribute * new_attr = new FwdAttribute(prop_list,attr,dev_name,i);
						add_attr(new_attr);
					}
					else
					{
						Attribute * new_attr = new WAttribute(prop_list, attr, dev_name, i);
						add_attr(new_attr);
					}
				}
				else
				{
					if (attr.is_fwd() == true)
					{
						Attribute * new_attr = new FwdAttribute(prop_list, attr, dev_name, i);
						add_attr(new_attr);
					}
					else
					{
						Attribute * new_attr = new Attribute(prop_list, attr, dev_name, i);
						add_attr(new_attr);
					}
				}

//
// If it is writable, add it to the writable attribute list
//

				Tango::AttrWriteType w_type = attr_list[i - sub]->get_writable();
				if ((w_type == Tango::WRITE) ||
					(w_type == Tango::READ_WRITE))
				{
					writable_attr_list.push_back(i - sub);
				}

//
// If one of the alarm properties is defined, add it to the alarmed attribute list
//

				if (attr_list[i - sub]->is_alarmed().any() == true)
				{
					if (w_type != Tango::WRITE)
						alarm_attr_list.push_back(i - sub);
				}

				cout4 << *(attr_list[i - sub]) << endl;
			}
			else
				sub++;
		}

//
// Remove attdesc entry in class object for forwarded attribute with root device not started
//

//		for (unsigned int loop = 0;loop < rem_att_name.size();loop++)
//		{
//			dev_class_ptr->get_class_attr()->remove_attr(rem_att_name[loop],rem_att_cl_name[loop]);
//		}

	}

//
// For each attribute, check if the writable_attr_name is set and in this case, check if the associated attribute
// exists and is writable
//

	nb_attr = attr_list.size();
	for (i = 0;i < nb_attr;i++)
	{
		check_associated(i,dev_name);
	}

	cout4 << "Leaving MultiAttribute class constructor" << endl;
}

//+----------------------------------------------------------------------------------------------------------------
//
// method :
//		MultiAttribute::~MultiAttribute
//
// description :
//		Destructor for the MultiAttribute class. It simply delete all the Attribute object stored in its
//		attr_list data member
//
//-----------------------------------------------------------------------------------------------------------------

MultiAttribute::~MultiAttribute()
{
	for(unsigned long i = 0;i < attr_list.size();i++)
		delete attr_list[i];
	ext->attr_map.clear();
#ifndef HAS_UNIQUE_PTR
	delete ext;
#endif
}

//+-----------------------------------------------------------------------------------------------------------------
//
// method :
//		MultiAttribute::concat
//
// description :
//		Concatenate porperties defined at the class level and at the device level. Prperties defined at the device
//		level have the highest level
//
// arguments:
// 		in :
//			- dev_prop : The device properties
//			- class_prop : The class properties
// 		out :
//			- result : The resulting vector
//
//------------------------------------------------------------------------------------------------------------------


void MultiAttribute::concat(vector<AttrProperty> &dev_prop,
						    vector<AttrProperty> &class_prop,
							vector<AttrProperty> &result)
{

//
// Copy all device properties
//

	unsigned long i;
	for (i = 0;i < dev_prop.size();i++)
		result.push_back(dev_prop[i]);

//
// Add class properties if they have not been redefined at the device level
//

	vector<AttrProperty> tmp_result = result;
	unsigned long nb_class_check = class_prop.size();
	for (i = 0;i < nb_class_check;i++)
	{
		vector<AttrProperty>::iterator pos;

		pos = find_if(tmp_result.begin(),tmp_result.end(),
			      bind2nd(WantedProp<AttrProperty,string,bool>(),class_prop[i].get_name()));

		if (pos != tmp_result.end())
			continue;
		else
			result.push_back(class_prop[i]);
	}
}

//+------------------------------------------------------------------------------------------------------------------
//
// method : 		MultiAttribute::add_default
//
// description :
//		Add default value for optional property if they are not defined
//
// argument :
// 		in :
//			- prop_list : The already defined property vector
//			- dev_name : The device name
//			- att_name : The attribute name
//			- att_data_type : The attribute data type
//
//--------------------------------------------------------------------------

void MultiAttribute::add_default(vector<AttrProperty> &prop_list, TANGO_UNUSED(string &dev_name),
				 string &att_name,long att_data_type)
{

//
// Overwrite the label attribute property in order to be set to the attribute name
//

	Tango_OptAttrProp[0].default_value = att_name.c_str();

//
// Also overwrite the format attribute property according to attribute data type
//

	switch (att_data_type)
	{
	case DEV_SHORT:
	case DEV_LONG:
	case DEV_LONG64:
	case DEV_UCHAR:
	case DEV_USHORT:
	case DEV_ULONG:
	case DEV_ULONG64:
		Tango_OptAttrProp[5].default_value = FormatNotSpec_INT;
		break;

	case DEV_STRING:
	case DEV_ENUM:
		Tango_OptAttrProp[5].default_value = FormatNotSpec_STR;
		break;

	case DEV_FLOAT:
	case DEV_DOUBLE:
		Tango_OptAttrProp[5].default_value = FormatNotSpec_FL;
		break;

	case DEV_STATE:
	case DEV_ENCODED:
	case DEV_BOOLEAN:
	case DATA_TYPE_UNKNOWN:
		Tango_OptAttrProp[5].default_value = AlrmValueNotSpec;
		break;

	default:
		break;
	}

	long nb_opt_prop = sizeof(Tango_OptAttrProp)/sizeof(OptAttrProp);

//
// For all the optional attribute properties, search in the already built vector of attributes if they are defined.
// If yes, continue. Otherwise, add a new property with the default value
//

	for (long i = 0;i < nb_opt_prop;i++)
	{
		vector<AttrProperty>::iterator pos;
		string opt_prop_name(Tango_OptAttrProp[i].name);

		pos = find_if(prop_list.begin(),prop_list.end(),
			      bind2nd(WantedProp<AttrProperty,string,bool>(),opt_prop_name));

		if (pos == prop_list.end())
			prop_list.push_back(AttrProperty(Tango_OptAttrProp[i].name,Tango_OptAttrProp[i].default_value));
	}
}

//+------------------------------------------------------------------------------------------------------------------
//
// method :
//		MultiAttribute::add_user_default
//
// description :
//		Add default value for optional property if they are not defined
//
// argument :
// 		in :
//			- prop_list : The already defined property vector
//			- user_default : The user defined default property values
//
//-------------------------------------------------------------------------------------------------------------------

void MultiAttribute::add_user_default(vector<AttrProperty> &prop_list,vector<AttrProperty> &user_default)
{

//
// Add default user properties if they have not been defined in the database
//

	long nb_user = user_default.size();
	for (int i = 0;i < nb_user;i++)
	{
		vector<AttrProperty>::iterator pos;

		pos = find_if(prop_list.begin(),prop_list.end(),
			      bind2nd(WantedProp<AttrProperty,string,bool>(),user_default[i].get_name()));

		if (pos != prop_list.end())
			continue;
		else
			prop_list.push_back(user_default[i]);
	}
}

//+-----------------------------------------------------------------------------------------------------------------
//
// method :
//		MultiAttribute::check_associated
//
// description :
//		Check if the writable_attr_name property is set and in this case, check if the associated attribute exists
//		and is writable. This is necessary only for attribute of the READ_WITH_WRITE or READ_WRITE types
//
// argument :
//		in :
//			- index : The index of the attribute to checked in the attr vector
//			- dev_name : The device name
//
//------------------------------------------------------------------------------------------------------------------

void MultiAttribute::check_associated(long index,string &dev_name)
{
	if ((attr_list[index]->get_writable() == Tango::READ_WITH_WRITE) ||
	    (attr_list[index]->get_writable() == Tango::READ_WRITE))
	{
		unsigned long j;
		string &assoc_name = attr_list[index]->get_assoc_name();
		transform(assoc_name.begin(),assoc_name.end(),assoc_name.begin(),::tolower);
		for (j = 0;j < writable_attr_list.size();j++)
		{
			if (attr_list[writable_attr_list[j]]->get_name_lower() == assoc_name)
				break;
		}
		if (j == writable_attr_list.size())
		{
			TangoSys_OMemStream o;

			o << "Device --> " << dev_name;
			o << "\nProperty writable_attr_name for attribute " << attr_list[index]->get_name();
			o << " is set to " << assoc_name;
			o << ", but this attribute does not exists or is not writable" << ends;
			Except::throw_exception((const char *)API_AttrOptProp,
						o.str(),
						(const char *)"MultiAttribute::MultiAttribute");
		}

//
// Check that the two associated attributes have the same data type
//

		if (attr_list[writable_attr_list[j]]->get_data_type() != attr_list[index]->get_data_type())
		{
			TangoSys_OMemStream o;

			o << "Device --> " << dev_name;
			o << "\nProperty writable_attr_name for attribute " << attr_list[index]->get_name();
			o << " is set to " << assoc_name;
			o << ", but these two attributes do not support the same data type" << ends;
			Except::throw_exception((const char *)API_AttrOptProp,
						o.str(),
						(const char *)"MultiAttribute::MultiAttribute");
		}

		attr_list[index]->set_assoc_ind(writable_attr_list[j]);
	}

}

//+-----------------------------------------------------------------------------------------------------------------
//
// method :
//		MultiAttribute::check_idl_release
//
// description :
//		Check some features according to which IDL release the device implement. This can not be done in
//		DeviceImpl class ctor because when the DeviceImpl ctor is executed, it is always IDL release 1.
//		The real supported IDL release number is set during each Device_XImpl ctor which is executed after
//		DeviceImpl ctor.
//		Today, we check :
//			- Enum attribute with IDL5
//			- Forwarded attribute with IDL 5
//
// argument :
//		in :
//			- dev : The device pointer
//
//------------------------------------------------------------------------------------------------------------------

void MultiAttribute::check_idl_release(DeviceImpl *dev)
{
	int idl_version = dev->get_dev_idl_version();
	size_t nb_attr = attr_list.size();
	bool vector_cleared = false;

	for (size_t i = 0;i < nb_attr;i++)
	{

//
// Check for enumerated attribute
//

		if (attr_list[i]->get_data_type() == DEV_ENUM && idl_version < 5)
		{
			try
			{
				stringstream ss;
				ss << "Attribute " << attr_list[i]->get_name() << " has a DEV_ENUM data type.\n";
				ss << "This is supported oonly for device inheriting from IDL 5 or more";

				Except::throw_exception(API_NotSupportedFeature,ss.str(),"MultiAttribute::check_idl_release()");
			}
			catch (Tango::DevFailed &e)
			{
				attr_list[i]->add_startup_exception("enum_labels",e);
			}
		}

//
// Chek for forwarded attribute
// If IDL < 5, on top of setting wrongly configured attribute in device, remove the root att from
// root attribute registry
//

		if (attr_list[i]->is_fwd_att() == true && idl_version < 5)
		{
			vector<DeviceImpl::FwdWrongConf> &fwd_wrong_conf = dev->get_fwd_att_wrong_conf();
			if (vector_cleared == false)
			{
				fwd_wrong_conf.clear();
				vector_cleared = true;
			}

			DeviceImpl::FwdWrongConf fwc;
			fwc.att_name = attr_list[i]->get_name();
			FwdAttribute *fwd_attr = static_cast<FwdAttribute *>(attr_list[i]);
			fwc.full_root_att_name = fwd_attr->get_fwd_dev_name() + '/' + fwd_attr->get_fwd_att_name();
			fwc.fae = FWD_TOO_OLD_LOCAL_DEVICE;
			fwd_wrong_conf.push_back(fwc);

			RootAttRegistry &fdp = Util::instance()->get_root_att_reg();
			fdp.remove_root_att(fwd_attr->get_fwd_dev_name(),fwd_attr->get_fwd_att_name());
		}
	}
}

//+-------------------------------------------------------------------------------------------------------------------
//
// method :
//		MultiAttribute::add_attribute
//
// description :
//		Construct a new attribute object and add it to the device attribute list
//
// argument :
//		in :
//			- dev_name : The device name
//			- dev_class_ptr : Pointer to the DeviceClass object
//			- index : Index in class attribute list of the new device attribute
//
//-------------------------------------------------------------------------------------------------------------------

void MultiAttribute::add_attribute(string &dev_name,DeviceClass *dev_class_ptr,long index)
{
	cout4 << "Entering MultiAttribute::add_attribute" << endl;

//
// Retrieve device class attribute list
//

	vector<Attr *> &tmp_attr_list = dev_class_ptr->get_class_attr()->get_attr_list();

//
// Get device attribute properties
// No need to implement a retry here (in case of db server restart) because the db reconnection
// is forced by the get_property call executed during xxxClass construction before we reach this code.
//

	Tango::Util *tg = Tango::Util::instance();
	Tango::DbData db_list;

	if (tg->_UseDb == true)
	{
		db_list.push_back(DbDatum(tmp_attr_list[index]->get_name()));

		try
		{
			tg->get_database()->get_device_attribute_property(dev_name,db_list,tg->get_db_cache());
		}
		catch (Tango::DevFailed &e)
		{
			TangoSys_OMemStream o;
			o << "Can't get device attribute properties for device " << dev_name << ends;

			Except::re_throw_exception(e,(const char *)API_DatabaseAccess,
				       		 o.str(),
				        	(const char *)"MultiAttribute::add_attribute");
		}
	}

//
// Get attribute class properties
//

	Attr &attr = dev_class_ptr->get_class_attr()->get_attr(tmp_attr_list[index]->get_name());
	vector<AttrProperty> &class_prop = attr.get_class_properties();
	vector<AttrProperty> &def_user_prop = attr.get_user_default_properties();

//
// If the attribute has some properties defined at device level, build a vector of these properties
//

	vector<AttrProperty> dev_prop;

	if (tg->_UseDb == true)
	{
	    long ind = 0;
		long nb_prop = 0;
		db_list[ind] >> nb_prop;
		ind++;

		for (long j = 0;j < nb_prop;j++)
		{
			if (db_list[ind].size() > 1)
			{
				string tmp(db_list[ind].value_string[0]);
				long nb = db_list[ind].size();
				for (int k = 1;k < nb;k++)
				{
                    tmp = tmp + ",";
					tmp = tmp + db_list[ind].value_string[k];
				}
				dev_prop.push_back(AttrProperty(db_list[ind].name,tmp));
			}
			else
				dev_prop.push_back(AttrProperty(db_list[ind].name,
								db_list[ind].value_string[0]));
			ind++;
		}
	}

//
// Concatenate these two attribute properties levels
//

	vector<AttrProperty> prop_list;
	concat(dev_prop,class_prop,prop_list);
	add_user_default(prop_list,def_user_prop);
	add_default(prop_list,dev_name,attr.get_name(),attr.get_type());

//
// Create an Attribute instance and insert it in the attribute list. If the device implement IDL 3
// (with state and status as attributes), add it at the end of the list but before state and status.
//

	bool idl_3 = false;
	vector<Attribute *>::iterator ite;
	if ((attr_list.back())->get_name() == "Status")
	{
		idl_3 = true;
		ite = attr_list.end();
		ite = ite - 2;
	}

	if ((attr.get_writable() == Tango::WRITE) ||
	    (attr.get_writable() == Tango::READ_WRITE))
	{
		if (idl_3 == false)
		{
			Attribute * new_attr = new WAttribute(prop_list,attr,dev_name,index);
			add_attr(new_attr);
			index = attr_list.size() - 1;
		}
		else
		{
			Attribute * new_attr = new WAttribute(prop_list,attr,dev_name,index);
			attr_list.insert(ite,new_attr);
			index = attr_list.size() - 3;
			ext->put_attribute_in_map(new_attr,index);
		}
	}
	else
	{
		if (idl_3 == false)
		{
			Attribute * new_attr = new Attribute(prop_list,attr,dev_name,index);
			add_attr(new_attr);
			index = attr_list.size() - 1;
		}
		else
		{
			Attribute * new_attr = new Attribute(prop_list,attr,dev_name,index);
			attr_list.insert(ite,new_attr);
			index = attr_list.size() - 3;
			ext->put_attribute_in_map(new_attr,index);
		}
	}

//
// If it is writable, add it to the writable attribute list
//

	Tango::AttrWriteType w_type = attr_list[index]->get_writable();
	if ((w_type == Tango::WRITE) ||
	    (w_type == Tango::READ_WRITE))
	{
		writable_attr_list.push_back(index);
	}

//
// If one of the alarm properties is defined, add it to the alarmed attribute list
//

	if (attr_list[index]->is_alarmed().any() == true)
	{
		if (w_type != Tango::WRITE)
			alarm_attr_list.push_back(index);
	}

//
// Check if the writable_attr_name property is set and in this case, check if the associated attribute exists and is
// writable
//

	check_associated(index,dev_name);

	cout4 << "Leaving MultiAttribute::add_attribute" << endl;
}

//+-------------------------------------------------------------------------------------------------------------------
//
// method :
//		MultiAttribute::add_fwd_attribute
//
// description :
//		Construct a new forwarded attribute object and add it to the device attribute list
//
// argument :
//		in :
//			- dev_name : The device name
//			- dev_class_ptr : Pointer to the DeviceClass object
//			- index : Index in class attribute list of the new device attribute
//
//-------------------------------------------------------------------------------------------------------------------

void MultiAttribute::add_fwd_attribute(string &dev_name,DeviceClass *dev_class_ptr,long index, Attr *new_attr)
{
	cout4 << "Entering MultiAttribute::add_fwd_attribute" << endl;

//
// Retrieve device class attribute list
//

	vector<Attr *> &tmp_attr_list = dev_class_ptr->get_class_attr()->get_attr_list();

//
// Get attribute class properties
//

	Attr &attr = dev_class_ptr->get_class_attr()->get_attr(tmp_attr_list[index]->get_name());
	vector<AttrProperty> &def_user_prop = attr.get_user_default_properties();

//
// Concatenate these two attribute properties levels
//

	vector<AttrProperty> prop_list;
	add_user_default(prop_list,def_user_prop);
	add_default(prop_list,dev_name,attr.get_name(),attr.get_type());

//
// Validate and register the root attribute configuration
//
	if (new_attr->is_fwd() == true)
	{
		Tango::FwdAttr *fwd_attr = (Tango::FwdAttr *) new_attr;
		// If forwarded attribute is dynamically created and was constructed without specifying
		// the root_attribute parameter then we have to get its __root_att property from tango DB
		// prior to calling validate_fwd_att()
		vector<AttrProperty> dev_prop;
		Tango::Util *tg = Tango::Util::instance();
		if ((tg->_UseDb==true) && (fwd_attr->get_full_root_att()==RootAttNotDef))
		{
			Tango::DbData db_list;
			db_list.push_back(DbDatum(fwd_attr->get_name()));
			tg->get_database()->get_device_attribute_property(dev_name,db_list,tg->get_db_cache());
			for (unsigned int ind=0 ; ind<db_list.size() ; ind++)
			{
				if (db_list[ind].name == RootAttrPropName)
				{
					dev_prop.push_back(AttrProperty(db_list[ind].name, db_list[ind].value_string[0]));
					break;
				}
			}
		}

		// Validate the __root_att property of the attribute
		fwd_attr->validate_fwd_att(dev_prop, dev_name);

		// Register the root attribute configuration
		Tango::DeviceImpl *device = tg->get_device_by_name(dev_name);
		try
		{
			fwd_attr->get_root_conf(dev_name,device);
		}
		catch (...)
		{
			// If any error add this attribute to the device list of wrong configured attributes
			DeviceImpl::FwdWrongConf fwc;
			fwc.att_name = fwd_attr->get_name();
			fwc.full_root_att_name = fwd_attr->get_full_root_att();
			fwc.fae = fwd_attr->get_err_kind();
			vector<DeviceImpl::FwdWrongConf> &fwd_att_wrong_conf = device->get_fwd_att_wrong_conf();
			fwd_att_wrong_conf.push_back(fwc);
		}
	}

//
// Create an Attribute instance and insert it in the attribute list. If the device implement IDL 3
// (with state and status as attributes), add it at the end of the list but before state and status.
//

	vector<Attribute *>::iterator ite;
	ite = attr_list.end() - 2;

	Attribute * new_fwd_attr = new FwdAttribute(prop_list,*new_attr,dev_name,index);
	attr_list.insert(ite,new_fwd_attr);
	index = attr_list.size() - 3;
	ext->put_attribute_in_map(new_fwd_attr,index);

//
// If it is writable, add it to the writable attribute list
//

	Tango::AttrWriteType w_type = attr_list[index]->get_writable();
	if ((w_type == Tango::WRITE) ||
	    (w_type == Tango::READ_WRITE))
	{
		writable_attr_list.push_back(index);
	}

//
// If one of the alarm properties is defined, add it to the alarmed attribute list
//

	if (attr_list[index]->is_alarmed().any() == true)
	{
		if (w_type != Tango::WRITE)
			alarm_attr_list.push_back(index);
	}

//
// Check if the writable_attr_name property is set and in this case, check if the associated attribute exists and is
// writable
//
	check_associated(index,dev_name);

	cout4 << "Leaving MultiAttribute::add_fwd_attribute" << endl;
}

//+------------------------------------------------------------------------------------------------------------------
//
// method :
//		MultiAttribute::remove_attribute
//
// description :
//		Remove one  attribute object from the device attribute list
//
// argument :
//		in :
//			- attr_name : The attribute name
//			- update_idx : Flag set to true if the attributes object index used to retrieve the corresponding Attr
//						   object has to be updated (because one Attr object has been removed)
//
//--------------------------------------------------------------------------------------------------------------------

void MultiAttribute::remove_attribute(string &attr_name,bool update_idx)
{
	cout4 << "Entering MultiAttribute::remove_attribute" << endl;

//
// Get attribute index in vector
//

	long att_index = get_attr_ind_by_name(attr_name.c_str());

//
// Remove the attribute from the main vector
//

	Attribute *att = attr_list[att_index];

	int old_idx = att->get_attr_idx();
	DeviceImpl *the_dev = att->get_att_device();
	string &dev_class_name = the_dev->get_device_class()->get_name();

	ext->attr_map.erase(att->get_name_lower());
	delete att;
	vector<Tango::Attribute *>::iterator pos = attr_list.begin();
	advance(pos,att_index);
	pos = attr_list.erase(pos);

//
// Update all the index for attribute following the one which has been deleted
// This is a 2 steps process:
// 1 - Update index in local device
// 2 - Update indexes in remaining device(s) belonging to the same class
//

	if (update_idx == true)
	{
		for (;pos != attr_list.end();++pos)
			(*pos)->set_attr_idx((*pos)->get_attr_idx() - 1);

		Tango::Util *tg = Tango::Util::instance();
		vector<DeviceImpl *> &dev_list = tg->get_device_list_by_class(dev_class_name);

		vector<DeviceImpl *>::iterator dev_ite;
		for (dev_ite = dev_list.begin();dev_ite != dev_list.end();++dev_ite)
		{
			if (*dev_ite == the_dev)
				continue;

			vector<Attribute *> &dev_att_list = (*dev_ite)->get_device_attr()->get_attribute_list();
			for (unsigned int loop = 0;loop < dev_att_list.size();++loop)
			{
				int idx = dev_att_list[loop]->get_attr_idx();
				if (idx > old_idx)
					dev_att_list[loop]->set_attr_idx(idx - 1);
			}
		}
	}

//
// Clear the writable attribute index vector and rebuild it. This is necessary because this vector containd index in
// the main attribute list which has been modified
//

	writable_attr_list.clear();
	for (unsigned long i = 0;i < attr_list.size();i++)
	{
		Tango::AttrWriteType w_type = attr_list[i]->get_writable();
		if ((w_type == Tango::WRITE) || (w_type == Tango::READ_WRITE))
		{
			writable_attr_list.push_back(i);
		}
	}

//
// Do the same for the alarmed attribute for the same reason
//

	alarm_attr_list.clear();
	for (unsigned long i = 0;i < attr_list.size();i++)
	{
		if (attr_list[i]->is_alarmed().any() == true)
		{
			Tango::AttrWriteType w_type = attr_list[i]->get_writable();
			if (w_type != Tango::WRITE)
				alarm_attr_list.push_back(i);
		}
	}

//
// Check the associated attributes
//

	string default_dev_name("a/b/c");
	for (unsigned long i = 0;i < attr_list.size();i++)
	{
		check_associated(i,default_dev_name);
	}

	cout4 << "Leaving MultiAttribute::remove_attribute" << endl;
}


//+-----------------------------------------------------------------------------------------------------------------
//
// method :
//		MultiAttribute::get_attr_by_name
//
// description :
//		Return a reference to the the Attribute object for the wanted attribue
//
// argument :
// 		in :
//			- attr_name : The attribute name
//
// return :
// 		A reference to the wanted attribute or throws an exception id the attribute is not found
//
//-------------------------------------------------------------------------------------------------------------------

Attribute &MultiAttribute::get_attr_by_name(const char *attr_name)
{
    Attribute * attr = 0;
    string st(attr_name);
    transform(st.begin(),st.end(),st.begin(),::tolower);
#ifdef HAS_MAP_AT
    try
    {
        attr = ext->attr_map.at(st).att_ptr;
    }
    catch(out_of_range e)
    {
        cout3 << "MultiAttribute::get_attr_by_name throwing exception" << endl;
        TangoSys_OMemStream o;

        o << attr_name << " attribute not found" << ends;
        Except::throw_exception((const char *)API_AttrNotFound,
                                o.str(),
                                (const char *)"MultiAttribute::get_attr_by_name");
    }
#else
    map<string, MultiAttributeExt::AttributePtrAndIndex>::iterator it;
    it = ext->attr_map.find(st);
    if (it == ext->attr_map.end())
    {
        cout3 << "MultiAttribute::get_attr_by_name throwing exception" << endl;
        TangoSys_OMemStream o;

        o << attr_name << " attribute not found" << ends;
        Except::throw_exception((const char *)API_AttrNotFound,
                                o.str(),
                                (const char *)"MultiAttribute::get_attr_by_name");
    }
    attr = it->second.att_ptr;
#endif
    return *attr;
}

//+------------------------------------------------------------------------------------------------------------------
//
// method :
//		MultiAttribute::get_w_attr_by_name
//
// description :
//		Return a reference to the the Attribute object for the wanted attribue
//
// argument :
// 		in :
//			- attr_name : The attribute name
//
// return :
//		A reference to the wanted attribute or throws an exception id the attribute is not found
//
//-------------------------------------------------------------------------------------------------------------------

WAttribute &MultiAttribute::get_w_attr_by_name(const char *attr_name)
{
    Attribute * attr = 0;
    string st(attr_name);
    transform(st.begin(),st.end(),st.begin(),::tolower);
#ifdef HAS_MAP_AT
    try
    {
        attr = ext->attr_map.at(st).att_ptr;
    }
    catch(out_of_range e)
    {
        cout3 << "MultiAttribute::get_attr_by_name throwing exception" << endl;
        TangoSys_OMemStream o;

        o << attr_name << " writable attribute not found" << ends;
        Except::throw_exception((const char *)API_AttrNotFound,
                                o.str(),
                                (const char *)"MultiAttribute::get_w_attr_by_name");
    }
#else
    map<string, MultiAttributeExt::AttributePtrAndIndex>::iterator it;
    it = ext->attr_map.find(st);
    if (it == ext->attr_map.end())
    {
        cout3 << "MultiAttribute::get_attr_by_name throwing exception" << endl;
        TangoSys_OMemStream o;

        o << attr_name << " writable attribute not found" << ends;
        Except::throw_exception((const char *)API_AttrNotFound,
                                o.str(),
                                (const char *)"MultiAttribute::get_w_attr_by_name");
    }
    attr = it->second.att_ptr;
#endif

    if ((attr->get_writable() != Tango::WRITE) &&
        (attr->get_writable() != Tango::READ_WRITE))
    {
        cout3 << "MultiAttribute::get_attr_by_name throwing exception" << endl;
        TangoSys_OMemStream o;

        o << attr_name << " writable attribute not found" << ends;
        Except::throw_exception((const char *)API_AttrNotFound,
                                o.str(),
                                (const char *)"MultiAttribute::get_w_attr_by_name");
    }
    return static_cast<WAttribute &>(*attr);
}

//+-------------------------------------------------------------------------------------------------------------------
//
// method :
//		MultiAttribute::get_attr_ind_by_name
//
// description :
//		Return the index in the Attribute object vector of a specified attribute
//
// argument :
// 		in :
//			- attr_name : The attribute name
//
// return :
//		The index of the wanted attribute or throws an exception id the attribute is not found
//
//--------------------------------------------------------------------------------------------------------------------

long MultiAttribute::get_attr_ind_by_name(const char *attr_name)
{
    long i;
    string st(attr_name);

    transform(st.begin(),st.end(),st.begin(),::tolower);
#ifdef HAS_MAP_AT
    try
    {
        i = ext->attr_map.at(st).att_index_in_vector;
    }
    catch(out_of_range e)
    {
        cout3 << "MultiAttribute::get_attr_ind_by_name throwing exception" << endl;
        TangoSys_OMemStream o;

        o << attr_name << " attribute not found" << ends;
        Except::throw_exception((const char *)API_AttrNotFound,
                                o.str(),
                                (const char *)"MultiAttribute::get_attr_ind_by_name");
    }
#else
    map<string, MultiAttributeExt::AttributePtrAndIndex>::iterator it;
    it = ext->attr_map.find(st);
    if (it == ext->attr_map.end())
    {
        cout3 << "MultiAttribute::get_attr_ind_by_name throwing exception" << endl;
        TangoSys_OMemStream o;

        o << attr_name << " attribute not found" << ends;
        Except::throw_exception((const char *)API_AttrNotFound,
                                o.str(),
                                (const char *)"MultiAttribute::get_attr_ind_by_name");
    }
    i = it->second.att_index_in_vector;
#endif
    return i;
}

//+--------------------------------------------------------------------------------------------------------------------
//
// method :
//		MultiAttribute::check_alarm
//
// description :
//		check alarm on all the attribute where one alarm is defined
//
// return :
//		A boolen set to true if one of the attribute with an alarm defined is in alarm state
//
//--------------------------------------------------------------------------------------------------------------------

bool MultiAttribute::check_alarm()
{
	unsigned long i;
	bool ret,tmp_ret;

	tmp_ret = false;
	ret = false;
	for (i = 0;i < alarm_attr_list.size();i++)
	{
		Tango::AttrQuality qua = (get_attr_by_ind(alarm_attr_list[i])).get_quality();
		if (ret == false)
		{
			if (qua == Tango::ATTR_ALARM)
				ret = true;
		}
		if (qua != Tango::ATTR_INVALID)
		{

//
// If the attribute is polled, the check_alarm method has already been called when the polling thread has read the
// attribute.
//

		    Attribute &att = get_attr_by_ind(alarm_attr_list[i]);
		    if (att.is_polled() == false)
		    {
                tmp_ret = check_alarm(alarm_attr_list[i]);
                if (tmp_ret == true)
                    ret = true;
		    }
		}
	}

	return ret;
}

//+------------------------------------------------------------------------------------------------------------------
//
// method :
//		MultiAttribute::read_alarm
//
// description :
//		Add a message in the device status string if one of the device attribute is in the alarm state
//
// argument :
// 		in :
//			- status : The device status
//
//--------------------------------------------------------------------------------------------------------------------

void MultiAttribute::read_alarm(string &status)
{
	unsigned long i;

	for (i = 0;i < alarm_attr_list.size();i++)
	{
		Attribute &att = get_attr_by_ind(alarm_attr_list[i]);

//
// Add a message for low level alarm
//

		if (att.is_min_alarm() == true)
		{
			string &attr_label = att.get_label();
			string str;
			if (attr_label == LabelNotSpec)
			{
				str = "\nAlarm : Value too low for attribute ";
				str = str + att.get_name();
			}
			else
			{
				str = "\nAlarm : Value too low for ";
				str = str + attr_label;
			}
			status = status + str;
		}

//
// Add a message for high level alarm
//

		else if (att.is_max_alarm() == true)
		{
			string &attr_label = att.get_label();
			string str;
			if (attr_label == LabelNotSpec)
			{
				str = "\nAlarm : Value too high for attribute ";
				str = str + att.get_name();
			}
			else
			{
				str = "\nAlarm : Value too high for ";
				str = str + attr_label;
			}
			status = status + str;
		}

//
// Add a message for rds alarm
//

		if (att.is_rds_alarm() == true)
		{
			string &attr_label = att.get_label();
			string str;
			if (attr_label == LabelNotSpec)
			{
				str = "\nAlarm : Read too Different than Set (RDS) for attribute ";
				str = str + att.get_name();
			}
			else
			{
				str = "\nAlarm : Read too Different than Set (RDS) for ";
				str = str + attr_label;
			}
			status = status + str;
		}

//
// Add a message for min warning
//

		if (att.is_min_warning() == true)
		{
			string &attr_label = att.get_label();
			string str;
			if (attr_label == LabelNotSpec)
			{
				str = "\nWarning : Value too low for attribute ";
				str = str + att.get_name();
			}
			else
			{
				str = "\nWarning : Value too low for ";
				str = str + attr_label;
			}
			status = status + str;
		}

//
// Add a message for max warning
//

		else if (att.is_max_warning() == true)
		{
			string &attr_label = att.get_label();
			string str;
			if (attr_label == LabelNotSpec)
			{
				str = "\nWarning : Value too high for attribute ";
				str = str + att.get_name();
			}
			else
			{
				str = "\nWarning : Value too high for ";
				str = str + attr_label;
			}
			status = status + str;
		}

	}
}

//+-----------------------------------------------------------------------------------------------------------------
//
// method :
//		MultiAttribute::get_event_param
//
// description :
//		Return event info for each attribute with events subscribed
//
// argument :
// 		in :
//			- eve : One structure in this vector for each attribute with events subscribed
//
//------------------------------------------------------------------------------------------------------------------

void MultiAttribute::get_event_param(vector<EventPar> &eve)
{
	unsigned int i;

	for (i = 0;i < attr_list.size();i++)
	{
		bool once_more = false;
		vector<int> ch;
		vector<int> ar;
		vector<int> pe;
		vector<int> us;
		vector<int> ac;
		bool dr = false;
		bool qu = false;

		if (attr_list[i]->change_event_subscribed() == true)
		{
			once_more = true;
			ch = attr_list[i]->get_client_lib(CHANGE_EVENT);
		}

		if (attr_list[i]->quality_event_subscribed() == true)
		{
			once_more = true;
			qu = true;
		}

		if (attr_list[i]->periodic_event_subscribed() == true)
		{
			once_more = true;
			pe = attr_list[i]->get_client_lib(PERIODIC_EVENT);
		}

		if (attr_list[i]->archive_event_subscribed() == true)
		{
			once_more = true;
			ar = attr_list[i]->get_client_lib(ARCHIVE_EVENT);
		}

		if (attr_list[i]->user_event_subscribed() == true)
		{
			once_more = true;
			us = attr_list[i]->get_client_lib(USER_EVENT);
		}

		if (attr_list[i]->attr_conf_event_subscribed() == true)
		{
			once_more = true;
			ac = attr_list[i]->get_client_lib(ATTR_CONF_EVENT);
		}

		if (attr_list[i]->data_ready_event_subscribed() == true)
		{
			once_more = true;
			dr = true;
		}

		if (once_more == true)
		{
			EventPar ep;

			if (attr_list[i]->use_notifd_event() == true)
                ep.notifd = true;
            else
                ep.notifd = false;

            if (attr_list[i]->use_zmq_event() == true)
                ep.zmq = true;
            else
                ep.zmq = false;

			ep.attr_id = i;
			ep.change = ch;
			ep.quality = qu;
			ep.archive = ar;
			ep.periodic = pe;
			ep.user = us;
			ep.att_conf = ac;
			ep.data_ready = dr;

			eve.push_back(ep);
		}
	}
}

//+-----------------------------------------------------------------------------------------------------------------
//
// method :
//		MultiAttribute::set_event_param
//
// description :
//		Set event info for each attribute with events subscribed
//
// argument :
// 		in :
//			- eve : One structure in this vector for each attribute with events subscribed
//
//------------------------------------------------------------------------------------------------------------------

void MultiAttribute::set_event_param(vector<EventPar> &eve)
{
	for (size_t i = 0;i < eve.size();i++)
	{
		if (eve[i].attr_id != -1)
		{
			Tango::Attribute &att = get_attr_by_ind(eve[i].attr_id);

			{
				omni_mutex_lock oml(EventSupplier::get_event_mutex());
				vector<int>::iterator ite;

				if (eve[i].change.empty() == false)
				{
					for (ite = eve[i].change.begin();ite != eve[i].change.end();++ite)
						att.set_change_event_sub(*ite);
				}

				if (eve[i].periodic.empty() == false)
				{
					for (ite = eve[i].periodic.begin();ite != eve[i].periodic.end();++ite)
						att.set_periodic_event_sub(*ite);
				}

				if (eve[i].archive.empty() == false)
				{
					for (ite = eve[i].archive.begin();ite != eve[i].archive.end();++ite)
						att.set_archive_event_sub(*ite);
				}

				if (eve[i].att_conf.empty() == false)
				{
					for (ite = eve[i].att_conf.begin();ite != eve[i].att_conf.end();++ite)
						att.set_att_conf_event_sub(*ite);
				}

				if (eve[i].user.empty() == false)
				{
					for (ite = eve[i].user.begin();ite != eve[i].user.end();++ite)
						att.set_user_event_sub(*ite);
				}

				if (eve[i].quality == true)
					att.set_quality_event_sub();
				if (eve[i].data_ready == true)
					att.set_data_ready_event_sub();
			}

			if (eve[i].notifd == true)
				att.set_use_notifd_event();
			if (eve[i].zmq == true)
				att.set_use_zmq_event();
		}
	}
}

//+------------------------------------------------------------------------------------------------------------------
//
// method :
//		MultiAttribute::add_write_value
//
// description :
//		For scalar attribute with an associated write attribute, the read_attributes CORBA operation also returns
//		the write value. This method gets the associated write attribute value and adds it to the read attribute
//
// argument :
// 		in :
//			- att : Reference to the attribute which must be read
//
//-------------------------------------------------------------------------------------------------------------------

void MultiAttribute::add_write_value(Attribute &att)
{
	WAttribute &assoc_att = get_w_attr_by_ind(att.get_assoc_ind());

	Tango::DevVarShortArray *sh_write_val;
	Tango::DevVarLongArray *lg_write_val;
	Tango::DevVarLong64Array *lg64_write_val;
	Tango::DevVarDoubleArray *db_write_val;
	Tango::DevVarStringArray *str_write_val;
	Tango::DevVarFloatArray *fl_write_val;
	Tango::DevVarBooleanArray *boo_write_val;
	Tango::DevVarUShortArray *ush_write_val;
	Tango::DevVarCharArray *uch_write_val;
	Tango::DevVarULongArray *ulg_write_val;
	Tango::DevVarULong64Array *ulg64_write_val;
	Tango::DevVarStateArray *state_write_val;

	switch (att.get_data_type())
	{
	case Tango::DEV_SHORT :
	case Tango::DEV_ENUM :
		sh_write_val = assoc_att.get_last_written_sh();
		att.add_write_value(sh_write_val);
		break;

	case Tango::DEV_LONG :
		lg_write_val = assoc_att.get_last_written_lg();
		att.add_write_value(lg_write_val);
		break;

	case Tango::DEV_LONG64 :
		lg64_write_val = assoc_att.get_last_written_lg64();
		att.add_write_value(lg64_write_val);
		break;

	case Tango::DEV_DOUBLE :
		db_write_val = assoc_att.get_last_written_db();
		att.add_write_value(db_write_val);
		break;

	case Tango::DEV_STRING :
		str_write_val = assoc_att.get_last_written_str();
		att.add_write_value(str_write_val);
		break;

	case Tango::DEV_FLOAT :
		fl_write_val = assoc_att.get_last_written_fl();
		att.add_write_value(fl_write_val);
		break;

	case Tango::DEV_BOOLEAN :
		boo_write_val = assoc_att.get_last_written_boo();
		att.add_write_value(boo_write_val);
		break;

	case Tango::DEV_USHORT :
		ush_write_val = assoc_att.get_last_written_ush();
		att.add_write_value(ush_write_val);
		break;

	case Tango::DEV_UCHAR :
		uch_write_val = assoc_att.get_last_written_uch();
		att.add_write_value(uch_write_val);
		break;

	case Tango::DEV_ULONG :
		ulg_write_val = assoc_att.get_last_written_ulg();
		att.add_write_value(ulg_write_val);
		break;

	case Tango::DEV_ULONG64 :
		ulg64_write_val = assoc_att.get_last_written_ulg64();
		att.add_write_value(ulg64_write_val);
		break;

	case Tango::DEV_STATE :
		state_write_val = assoc_att.get_last_written_state();
		att.add_write_value(state_write_val);
		break;

	case Tango::DEV_ENCODED :
		{
			DevEncoded &enc_write_val = assoc_att.get_last_written_encoded();
			att.add_write_value(enc_write_val);
			break;
		}
	}
}

//+------------------------------------------------------------------------------------------------------------------
//
// method :
//		MultiAttribute::is_att_quality_alarmed()
//
// description :
//		Check for all attribute if one of them has its quality factor set to ALARM.
//		Returns true in this case. Otherwise, returns false
//
//------------------------------------------------------------------------------------------------------------------

bool MultiAttribute::is_att_quality_alarmed()
{
	unsigned long i;
	bool ret;

	ret = false;

	for (i = 0;i < attr_list.size();i++)
	{
		if ((attr_list[i]->get_quality() == Tango::ATTR_ALARM) ||
		    (attr_list[i]->get_quality() == Tango::ATTR_WARNING))
		{
			ret = true;
			break;
		}
	}

	return ret;
}


//+------------------------------------------------------------------------------------------------------------------
//
// method :
//		MultiAttribute::add_alarmed_quality_factor()
//
// description :
//		Add to the status string name of attributes with a quality factor set to alarm
//
//-------------------------------------------------------------------------------------------------------------------

void MultiAttribute::add_alarmed_quality_factor(string &status)
{
	unsigned long i,j;

	for (i = 0;i < attr_list.size();i++)
	{
		bool found = false;
		for (j = 0;j < alarm_attr_list.size();j++)
		{
			if (alarm_attr_list[j] == (long)i)
			{
				found = true;
				break;
			}
		}
		if (found == true)
			continue;

		if (attr_list[i]->get_quality() == Tango::ATTR_ALARM)
		{
			string &attr_label = attr_list[i]->get_label();
			string str("\nAlarm : Quality factor set to ALARM for attribute ");

			if (attr_label == LabelNotSpec)
			{
				str = str + attr_list[i]->get_name();
			}
			else
			{
				str = str + attr_label;
			}
			status = status + str;
		}
		else if (attr_list[i]->get_quality() == Tango::ATTR_WARNING)
		{
			string &attr_label = attr_list[i]->get_label();
			string str("\nWarning : Quality factor set to WARNING for attribute ");

			if (attr_label == LabelNotSpec)
			{
				str = str + attr_list[i]->get_name();
			}
			else
			{
				str = str + attr_label;
			}
			status = status + str;
		}
	}
}

//+------------------------------------------------------------------------------------------------------------------
//
// method :
//		MultiAttribute::add_attr()
//
// description :
//		Add given attribute to the attribute list vector and map
//
// argument:
//		in :
//			- att : The newly configured attribute
//
//-------------------------------------------------------------------------------------------------------------------
void MultiAttribute::add_attr(Attribute *att)
{
    attr_list.push_back(att);
    ext->put_attribute_in_map(att,attr_list.size() - 1);
}

//+------------------------------------------------------------------------------------------------------------------
//
// method :
//		MultiAttribute::update()
//
// description :
//		Update attribute lists when a forwarded attribute conf. is now completely known (after a device with fwd
//		attributes started before the device with root attribute(s))
//
// argument:
//		in :
//			- att : The newly configured attribute
//			- dev_name : The device name
//
//-------------------------------------------------------------------------------------------------------------------

void MultiAttribute::update(Attribute &att,string &dev_name)
{
	long ind = get_attr_ind_by_name(att.get_name().c_str());

//
// If it is writable, add it to the writable attribute list if not there already
//

	Tango::AttrWriteType w_type = att.get_writable();
	if ((w_type == Tango::WRITE) ||
		(w_type == Tango::READ_WRITE))
	{
		vector<long>::iterator pos;
		pos = find(writable_attr_list.begin(),writable_attr_list.end(),ind);
		if (pos == writable_attr_list.end())
			writable_attr_list.push_back(ind);
	}

//
// If one of the alarm properties is defined, add it to the alarmed attribute list
//

	if (att.is_alarmed().any() == true)
	{
		if (w_type != Tango::WRITE)
		{
			vector<long>::iterator pos;
			pos = find(alarm_attr_list.begin(),alarm_attr_list.end(),ind);
			if (pos == alarm_attr_list.end())
				alarm_attr_list.push_back(ind);
		}
	}

//
// If if there is one associated att
//

	check_associated(ind,dev_name);
}

//+------------------------------------------------------------------------------------------------------------------
//
// method :
//		MultiAttribute::is_opt_prop()
//
// description :
//		This method returns true if the property name passed as argument is the name of one of the attribute optional
//      properties
//
// argument:
//		in :
//			- prop_name : The property name
//
// return:
//      True if prop_name is the name of one of the optional properties. False otherwise
//
//-------------------------------------------------------------------------------------------------------------------

bool MultiAttribute::is_opt_prop(const string &prop_name)
{
    bool ret = false;

	long nb_opt_prop = sizeof(Tango_OptAttrProp)/sizeof(OptAttrProp);

	for (long i = 0;i < nb_opt_prop;i++)
	{
	    if (prop_name == Tango_OptAttrProp[i].name)
        {
            ret = true;
            break;
        }
	}

    return ret;
}

} // End of Tango namespace
