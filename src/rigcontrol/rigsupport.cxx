// ----------------------------------------------------------------------------
// rigsupport.cxx - support functions file
//
// Copyright (C) 2007-2009
//		Dave Freese, W1HKJ
// Copyright (C) 2008-2009
//		Stelios Bounanos, M0GLD
// Copyright (C) 2013
//              Remi Chateauneu, F4ECW
//
// This file is part of fldigi.
//
// Fldigi is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Fldigi is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with fldigi.  If not, see <http://www.gnu.org/licenses/>.
// ----------------------------------------------------------------------------

#include <config.h>

#include <fstream>
#include <sstream>
#include <iostream>
#include <stdexcept>
#include <vector>
#include <list>
#include <map>
#include <algorithm>
#include <iterator>
#include <cstring>

#include "logsupport.h"
#include "rigsupport.h"
#include "rigxml.h"
#include "rigio.h"
#include "threads.h"
#include "record_loader.h"
#include "main.h"
#include "fl_digi.h"
#include "fileselect.h"
#include "trx.h"
#include "strutil.h"
#include "coordinate.h"

#include "configuration.h"

#include "globals.h"

#include "debug.h"

#include "gettext.h"

LOG_FILE_SOURCE(debug::LOG_RIGCONTROL);

using namespace std;

string windowTitle;

typedef vector<qrg_mode_t> FreqListType ;
static FreqListType freqlist;

static void remove_source( qrg_mode_t::FreqSource src )
{
	FreqListType::iterator en = std::remove_if(
			freqlist.begin(),
			freqlist.end(),
			std::bind2nd( std::mem_fun_ref( & qrg_mode_t::is_source ), src ) );
	LOG_INFO("Removing %d elements, from %d", (int)std::distance( en, freqlist.end() ), freqlist.size() );
	freqlist.erase( en, freqlist.end() );
}

static const unsigned char nfields = 4;
enum { max_rfcarrier, max_rmode, max_mode };

#if !USE_HAMLIB

typedef enum {
	RIG_MODE_NONE =  	0,	/*!< '' -- None */
	RIG_MODE_AM =    	(1<<0),	/*!< \c AM -- Amplitude Modulation */
	RIG_MODE_CW =    	(1<<1),	/*!< \c CW -- CW "normal" sideband */
	RIG_MODE_USB =		(1<<2),	/*!< \c USB -- Upper Side Band */
	RIG_MODE_LSB =		(1<<3),	/*!< \c LSB -- Lower Side Band */
	RIG_MODE_RTTY =		(1<<4),	/*!< \c RTTY -- Radio Teletype */
	RIG_MODE_FM =    	(1<<5),	/*!< \c FM -- "narrow" band FM */
	RIG_MODE_WFM =   	(1<<6),	/*!< \c WFM -- broadcast wide FM */
	RIG_MODE_CWR =   	(1<<7),	/*!< \c CWR -- CW "reverse" sideband */
	RIG_MODE_RTTYR =	(1<<8),	/*!< \c RTTYR -- RTTY "reverse" sideband */
	RIG_MODE_AMS =    	(1<<9),	/*!< \c AMS -- Amplitude Modulation Synchronous */
	RIG_MODE_PKTLSB =       (1<<10),/*!< \c PKTLSB -- Packet/Digital LSB mode (dedicated port) */
	RIG_MODE_PKTUSB =       (1<<11),/*!< \c PKTUSB -- Packet/Digital USB mode (dedicated port) */
	RIG_MODE_PKTFM =        (1<<12),/*!< \c PKTFM -- Packet/Digital FM mode (dedicated port) */
	RIG_MODE_ECSSUSB =      (1<<13),/*!< \c ECSSUSB -- Exalted Carrier Single Sideband USB */
	RIG_MODE_ECSSLSB =      (1<<14),/*!< \c ECSSLSB -- Exalted Carrier Single Sideband LSB */
	RIG_MODE_FAX =          (1<<15),/*!< \c FAX -- Facsimile Mode */
	RIG_MODE_SAM =          (1<<16),/*!< \c SAM -- Synchronous AM double sideband */
	RIG_MODE_SAL =          (1<<17),/*!< \c SAL -- Synchronous AM lower sideband */
	RIG_MODE_SAH =          (1<<18),/*!< \c SAH -- Synchronous AM upper (higher) sideband */
	RIG_MODE_DSB =			(1<<19), /*!< \c DSB -- Double sideband suppressed carrier */
} rmode_t;

#endif

static const struct rmode_name_t {
	rmode_t mode;
	const char *name;
} modes[] = {
	{ RIG_MODE_NONE, "NONE" },
	{ RIG_MODE_AM, "AM" },
	{ RIG_MODE_CW, "CW" },
	{ RIG_MODE_USB, "USB" },
	{ RIG_MODE_LSB, "LSB" },
	{ RIG_MODE_RTTY, "RTTY" },
	{ RIG_MODE_FM, "FM" },
	{ RIG_MODE_WFM, "WFM" },
	{ RIG_MODE_CWR, "CWR" },
	{ RIG_MODE_RTTYR, "RTTYR" },
	{ RIG_MODE_AMS, "AMS" },
	{ RIG_MODE_PKTLSB, "PKTLSB" },
	{ RIG_MODE_PKTUSB, "PKTUSB" },
	{ RIG_MODE_PKTFM, "PKTFM" }
//, // C99 trailing commas in enumerations not yet in the C++ standard
//	{ RIG_MODE_ECSSUSB, "ECSSUSB" },
//	{ RIG_MODE_ECSSLSB, "ECSSLSB" },
//	{ RIG_MODE_FAX, "FAX" }
// the above are covered by our requirement that hamlib be >= 1.2.4
#if (defined(RIG_MODE_SAM) && defined(RIG_MODE_SAL) && defined(RIG_MODE_SAH))
	, // C99 trailing commas in enumerations not yet in the C++ standard
	{ RIG_MODE_SAM, "SAM" },
	{ RIG_MODE_SAL, "SAL" },
	{ RIG_MODE_SAH, "SAH" }
#endif
};

static map<string, rmode_t> mode_nums;
static map<rmode_t, string> mode_names;

void qso_selMode(rmode_t m)
{
	qso_opMODE->value(mode_names[m].c_str());
}

const string & modeString(rmode_t m)
{
	return mode_names[m];
}

static void initOptionMenus()
{
	qso_opMODE->clear();
	list<RIGMODE>::iterator MD;
	list<RIGMODE> *pMD = 0;
	if (lmodes.empty() == false)
		pMD = &lmodes;
	else if (lmodeCMD.empty() == false)
		pMD = &lmodeCMD;

	if (pMD) {
		MD = pMD->begin();
		while (MD != pMD->end()) {
			qso_opMODE->add( (*MD).SYMBOL.c_str());
			MD++;
		}
		qso_opMODE->activate();
		qso_opMODE->index(0);
	}
	else {
		qso_opMODE->deactivate();
	}

	qso_opBW->clear();
	list<BW>::iterator bw;
	list<BW> *pBW = 0;
	if (lbws.empty() == false)
		pBW = &lbws;
	else if (lbwCMD.empty() == false)
		pBW = &lbwCMD;

	if (pBW) {
		bw = pBW->begin();
		while (bw != pBW->end()) {
			qso_opBW->add( (*bw).SYMBOL.c_str());
			bw++;
		}
		qso_opBW->activate();
		qso_opBW->index(0);
	}
	else {
		qso_opBW->deactivate();
	}
}

void clearList()
{
	freqlist.clear();
	qso_opBrowser->clear();
}

/// Copies the frequency list into the GUI browers.
static void updateSelect()
{
	LOG_INFO("Sz=%d", freqlist.size() );
	/// Reloaded from scratch.
	qso_opBrowser->clear();
	for (size_t i = 0, nb = freqlist.size() ; i < nb; i++) {
		qso_opBrowser->add(freqlist[i].str().c_str());
	}
	qso_displayFreq( wf->rfcarrier() );
}

/// Sorts and deduplicates the frequency list.
static void readFreqSortAndDedupl()
{
	sort(freqlist.begin(), freqlist.end());

	/// Detect duplicates using frequency and Levenshtein distance.
	int idx = 1, unidx = 0 ;

	/// We should check the similarity of all identical frequencies.
	for( int nb = freqlist.size(); idx < nb ; ++idx )
	{
		if(	( freqlist[idx].rfcarrier == freqlist[unidx].rfcarrier )
		&&	( freqlist[idx].mode      == freqlist[unidx].mode )
		&&	( freqlist[idx].rmode     == freqlist[unidx].rmode )
		&&	( levenshtein( freqlist[idx].name, freqlist[unidx].name ) < 2 ) ) {
			continue ;
		}
		++unidx ;
		if( unidx != idx ) freqlist[unidx] = freqlist[idx];
	}
	freqlist.resize(unidx+1);
	LOG_INFO("Sorted frequency file, %d unique records out of %d", unidx, idx );
}

static void strconcat( std::string & target, const std::string & source )
{
	if( source.empty() ) return ;

	if( target.empty() )
		target = source ;
	else {
		target += "," ;
		target += source ;
	}
}

/// Will be sliced when inserted into a frequency list.
class RecordEIBI : public qrg_mode_t
{
	/// From the CVS file.
	static const char m_delim = ';';
public:
	friend std::istream & operator>>( std::istream & istrm, RecordEIBI & rec )
	{
		/// Explanations here: http://www.eibispace.de/dx/README.TXT
		std::string ITU_cntry;

		/// One-letter or two-letter codes for different transmitter sites within one country.
		std::string trx_site_code ;

		// kHz:75;Time(UTC):93;Days:59;ITU:49;Station:201;Lng:49;Target:62;Remarks:135;P:35;Start:60;Stop:60;
		// 16.4;0000-2400;;NOR;JXN Marine Norway;;NEu;no;1;;
		double khz ;
		if( read_until_delim( m_delim, istrm, khz               )
		&&  read_until_delim( m_delim, istrm  /* time */        )
		&&  read_until_delim( m_delim, istrm  /* days */        ) // "24Dec", "Sa-Mo", "Tu-Fr", "Su-Th", "Su"
		&&  read_until_delim( m_delim, istrm, ITU_cntry         )
		&&  read_until_delim( m_delim, istrm, rec.name          )
		&&  read_until_delim( m_delim, istrm  /* language */    )
		&&  read_until_delim( m_delim, istrm  /* target */      )
		&&  read_until_delim( m_delim, istrm, trx_site_code     ) // Transmitter site code.
		&&  read_until_delim( m_delim, istrm  /* start */       ) // 2810
		&&  read_until_delim( m_delim, istrm  /* stop */        ) // 3112
		) 
		{
			rec.rfcarrier = 1000.0 * khz ;

			/// Beneficial because of reference counting.
			static const std::string rmode_none("AM");
			rec.rmode = rmode_none ;

			rec.carrier = 0 ;
			rec.mode = MODE_NULL;

			rec.description = ITU_cntry ;
			strconcat( rec.description, trx_site_code );

			/// This is loaded from a fixed file, so no need to save it.
			rec.data_source = FREQS_EIBI;
			return istrm ;
		}

		istrm.setstate(std::ios::badbit);
		return istrm ;
	}
}; // RecordEIBI

/// Will be sliced when inserted into a frequency list.
class RecordMWList : public qrg_mode_t
{
	/// From the CVS file.
	static const char m_delim = '|';

public:
	// MHZ|ITU|Program|Location|Region|Power|ID|latitude|longitude|schedule|la
	// 16.400000|NOR|JXN|Gildeskål|no|45.000000|86009|66.982778|13.873056|0000-2200||
	// 23.400000|D|DHO38|Rhauderfehn Marinefunksendestelle|nds||86043|53.081944|7.616389|24h||
	// 171.000000|RUS|Voice of Russia/R.Chechnya Svobodnaya|Tbilisskaya|KDA|1200.000000|18|45.485431|40.089333|0200-2000|ce,ru|
	friend std::istream & operator>>( std::istream & istrm, RecordMWList & rec )
	{
		std::string ITU_cntry, location, region;

		/// One-letter or two-letter codes for different transmitter sites within one country.
		std::string trx_site_code ;

		/// Coordinates are optional. We could use also read_until_delim with a default value.
		// nullable_t<double> latitude, longitude ;
		// double latitude = FP_NAN, longitude = FP_NAN;

		// kHz:75;Time(UTC):93;Days:59;ITU:49;Station:201;Lng:49;Target:62;Remarks:135;P:35;Start:60;Stop:60;
		// 16.4;0000-2400;;NOR;JXN Marine Norway;;NEu;no;1;;
		double khz ;

		if( read_until_delim( m_delim, istrm, khz                             )
		&&  read_until_delim( m_delim, istrm, ITU_cntry                       )
		&&  read_until_delim( m_delim, istrm, rec.name                        )
		&&  read_until_delim( m_delim, istrm, location                        )
		&&  read_until_delim( m_delim, istrm, region                          )
		&&  read_until_delim( m_delim, istrm  /* power */                     )
		&&  read_until_delim( m_delim, istrm  /* ID */                        )
		&&  read_until_delim( m_delim, istrm  /* latitude or ""= NAN  */      )
		&&  read_until_delim( m_delim, istrm  /* longitude or ""= NAN */      )
		&&  read_until_delim( m_delim, istrm  /* time */                      ) // "0200-2000", "24h"
		&&  read_until_delim( m_delim, istrm  /* language */                  )

		) 
		{
			rec.rfcarrier = 1000.0 * khz ;

			/// Beneficial because of reference counting.
			static const std::string rmode_none("AM");
			rec.rmode = rmode_none ;

			rec.carrier = 0 ;
			rec.mode = MODE_NULL;

			rec.description = location ;
			strconcat( rec.description, region );
			strconcat( rec.description, ITU_cntry );

			/// This is loaded from a fixed file, so no need to save it.
			rec.data_source = FREQS_MWLIST;

			/// Not used yet.
			CoordinateT::Pair station_coordinates ;

			return istrm ;
		}

		istrm.setstate(std::ios::badbit);
		return istrm ;
	}
}; // RecordMWList

/// Actually inserts a new frequency in the list and returns its index.
static size_t reorderList( const qrg_mode_t & m )
{
        freqlist.push_back(m);
        sort(freqlist.begin(), freqlist.end());

	/// The frequencies are sorted so no need to iterate.
        FreqListType::const_iterator pos = lower_bound(freqlist.begin(), freqlist.end(), m);
        if( (pos != freqlist.end()) && (m == *pos) )
                return pos - freqlist.begin();
        else
                return 0;
}

/// Adds a new frequency to the list and the browser. reorder things.
static size_t updateList(
	long long      rf,
	int            freq,
	const char   * rmd,
	trx_mode       md,
	const char   * nam = NULL,
	const char   * dsc = NULL)
{
	return reorderList( qrg_mode_t( rf, rmd, freq, md, nam, dsc ) );
}

/// val is rfcarrier. We might add fields from the ADIF edition.
static size_t addtoList(long long val)
{
	/// We use this dummy object so that members get the right default value.
        qrg_mode_t m(val);

        if (strlen(qso_opMODE->value()))
                m.rmode = qso_opMODE->value();

        if (active_modem) {
                m.carrier = active_modem->get_freq();
                m.mode = active_modem->get_mode();
        }

	/// These fields come from the ADIF editor.
	m.name = inpCall1->value();
	m.description = inpName1->value();
	m.data_source = qrg_mode_t::FREQS_NONE ;
        return reorderList(m);
}

/// Path to the file containing the frequency list.
static std::string HomeDirFreqFile()
{
	return HomeDir + "frequencies2.txt";
}

/// Reads a space-separated file containing one frequency (Radio station etc...) per line.
static bool readFreqListBeforeSort(const std::string & filname)
{
	LOG_INFO("Loading frequency file:%s", filname.c_str() );
	ifstream freqfile( filname.c_str() );
	if (!freqfile)
		return false;

	string line;
	qrg_mode_t m;
	while (!getline(freqfile, line).eof()) {
		if (line[0] == '#')
			continue;
		istringstream is(line);
		is >> m;
		freqlist.push_back(m);
	}
	freqfile.close();
	return true ;
}

static bool readFreqList(const std::string & filname)
{
	if( ! readFreqListBeforeSort(filname) ) return false ;
	readFreqSortAndDedupl();
	updateSelect();
	return true ;
}

void saveFreqList()
{

	ofstream freqfile(HomeDirFreqFile().c_str());
	if (!freqfile)
		return;
	freqfile << "# rfcarrier rig_mode carrier mode\n";

	if (freqlist.empty()) {
		freqfile.close();
		return;
	}

	for( FreqListType::const_iterator beg = freqlist.begin(), en = freqlist.end(); beg != en; ++beg )
	{
		if( beg->is_source( qrg_mode_t::FREQS_NONE ) ) {
			freqfile << *beg << "\n";
		}
	}
	freqfile.close();
}

/// Builds an initial file of well known frequencies.
static void buildlist()
{
	// find mode with longest shortname
	size_t s, smax = 0;
	for (size_t i = 0; i < NUM_MODES; i++) {
		s = strlen(mode_info[i].sname);
		if (smax < s) {
			smax = s;
		}
	}

	if (readFreqList( HomeDirFreqFile() ) == true)
		return;
        updateList ( 1807000L, 1000, "USB", MODE_PSK31,      "", "Common frequency, USA" );
        updateList ( 3505000L,  800, "USB", MODE_CW);
        updateList ( 3580000L, 1000, "USB", MODE_PSK31,      "", "Common operating frequency" );
        updateList ( 1000500L,  800, "USB", MODE_CW);
        updateList (10135000L, 1000, "USB", MODE_PSK31 );
        updateList ( 7005000L,  800, "USB", MODE_CW);
        updateList ( 7030000L, 1000, "USB", MODE_PSK31 );
        updateList ( 7070000L, 1000, "USB", MODE_PSK31,      "", "Common frequency, USA" );
        updateList (14005000L,  800, "USB", MODE_CW);
        updateList (14070000L, 1000, "USB", MODE_PSK31 );
        updateList (18100000L, 1000, "USB", MODE_PSK31 );
        updateList (21005000L,  800, "USB", MODE_CW);
        updateList (21070000L, 1000, "USB", MODE_PSK31 );
        updateList (24920000L, 1000, "USB", MODE_PSK31 );
        updateList (28005000L,  800, "USB", MODE_CW);
        updateList ( 28120000, 1000, "USB", MODE_PSK31 );
	updateList (   424000, 1000, "USB", MODE_NAVTEX,     "Navtex", _("Japan") );
	updateList (   490000, 1000, "USB", MODE_NAVTEX,     "Navtex", _("Local") );
	updateList (   518000, 1000, "USB", MODE_NAVTEX,     "Navtex", _("International") );
	updateList (  4209500, 1000, "USB", MODE_NAVTEX,     "Navtex", _("Tropical") );
	updateList (   145300, 1000, "USB", MODE_RTTY,       "DDH47", "FM12 / FM13 Synop Code" );
	updateList (  4582000, 1000, "USB", MODE_RTTY,       "DDK2", "FM12 / FM13 Synop Code" );
	updateList (  7645000, 1000, "USB", MODE_RTTY,       "DDH7", "FM12 / FM13 Synop Code" );
	updateList ( 10099800, 1000, "USB", MODE_RTTY,       "DDK9", "FM12 / FM13 Synop Code" );
	updateList ( 11038000, 1000, "USB", MODE_RTTY,       "DDH9", "FM12 / FM13 Synop Code" );
	updateList ( 14466300, 1000, "USB", MODE_RTTY,       "DDH8", "FM12 / FM13 Synop Code" );
	updateList (  2617500, 1000, "USB", MODE_WEFAX_576,  "Meteo Northwood (UK)" );
	updateList (  3854000, 1000, "USB", MODE_WEFAX_576,  "DDH3", "DWD (Deutscher Wetterdienst)" );
	updateList (  4609000, 1000, "USB", MODE_WEFAX_576,  "", "Meteo Northwood (UK)" );
	updateList (  5849000, 1000, "USB", MODE_WEFAX_576,  "", "Meteo Copenhagen (Dänemark)" );
	updateList (  7879000, 1000, "USB", MODE_WEFAX_576,  "DDK3", "DWD (Deutscher Wetterdienst)" );
	updateList (  8039000, 1000, "USB", MODE_WEFAX_576,  "", "Meteo Northwood (UK)" );
	updateList (  9359000, 1000, "USB", MODE_WEFAX_576,  "", "Meteo Copenhagen (Dänemark)" );
	updateList ( 11085500, 1000, "USB", MODE_WEFAX_576,  "", "Meteo Northwood (UK)" );
	updateList ( 13854000, 1000, "USB", MODE_WEFAX_576,  "", "Meteo Copenhagen (Dänemark)" );
	updateList ( 13881500, 1000, "USB", MODE_WEFAX_576,  "DDK6", "DWD (Deutscher Wetterdienst)" );
	updateList ( 17509000, 1000, "USB", MODE_WEFAX_576,  "", "Meteo Copenhagen (Dänemark)" );

	updateSelect();
}

int cb_qso_opMODE()
{
#if USE_HAMLIB
	if (progdefaults.chkUSEHAMLIBis) {
		if( 1 != hamlib_setmode(mode_nums[qso_opMODE->value()]) ) {
			LOG_WARN("Invalid qso mode:%s",qso_opMODE->value() );
		}
	}
	else
#endif
		rigCAT_setmode(qso_opMODE->value());
	return 0;
}

#define KRP_SUFFIX "krp"
#define TXT_SUFFIX "txt"

/// Tells if a filename is ended by a given extension.
static bool has_ext( const char * filnam, const char * filext )
{
	int lennam = strlen(filnam);
	int lenext = strlen(filext);

	return ( 0 == strcmp( filnam + lennam - lenext, filext ) );
}

/// When clicking to load a frequency list file.
void cb_LoadFreqList( Fl_Widget *, void *)
{
	static const char * filters =
		"ADIF\t*."   ADIF_SUFFIX "\n"
		"KRadio\t*." KRP_SUFFIX  "\n"
		"Text\t*."   TXT_SUFFIX 
#ifdef __APPLE__
		"\n"
#endif
		;

	const char* filnam = FSEL::select( _("Open frequency list"), filters );
	if (filnam) {
		if( has_ext(filnam, ADIF_SUFFIX) ) {
/*
			cAdifIO tmpFile;
			cQsoDb tmpDb ;
			tmp.do_readfile(filnam.c_str(), &tmpDb );
*/
			std::cout << "Adif file\n";
		} else
		if( has_ext(filnam, KRP_SUFFIX) ) {
			std::cout << "Krp file\n";
		} else
		if( has_ext(filnam, TXT_SUFFIX) ) {
			std::cout << "Text file\n";
			readFreqListBeforeSort( filnam );
		} else {
			LOG_ERROR("Unexpected file extension %s",filnam);
			return ;
		}

		/// Uses the same sort and deduplication for all sorts of new data.
		readFreqSortAndDedupl();
		updateSelect();
	}
}


/// This loads the EIBI stations file, about 11000 records.
struct LoaderEIBI : public RecordLoader< LoaderEIBI >
{
	FreqListType m_freqs_eibi;

	void Clear() {
		m_freqs_eibi.clear();
	}

	/// No intermediate storage.
	bool ReadRecord( std::istream & istrm ) {
		RecordEIBI tmp ;
		istrm >> tmp ;
		if( istrm || istrm.eof() ) {
			m_freqs_eibi.push_back( tmp );
			return true ;
		}
		return false ;
	}

	/// This file is also installed in data directory. Name changes with the year.
	const char * Url(void) const {
		return "http://www.eibispace.de/dx/sked-b12.csv";
	}

	const char * Description(void) const {
		return _("EiBi shortwaves");
	}
};

/// Updates the frequencies which are visible or not, depending on the flag.
void cb_EIBI( Fl_Widget *, void *)
try
{
	/// Depending on this flag, frequencies loaded from a fixed file are visible or not.
	static bool flagEIBI = false ;

	flagEIBI = ! flagEIBI ;

	LOG_INFO("flagEIBI=%d", (int)flagEIBI );

	LoaderEIBI & myLoader = LoaderEIBI::InstCatalog();

	static bool triedLoadEIBI = false ;
	if( flagEIBI ) {
		if( triedLoadEIBI == false ) {
			triedLoadEIBI = true ;
			int nbRec = myLoader.LoadAndRegister();
			if(nbRec < 0) {
				LOG_WARN("Error loading %s", myLoader.Url() );
				return ;
			}
		}
		LOG_INFO("Adding %d elements to %d", (int)myLoader.m_freqs_eibi.size(), (int)freqlist.size() );
		freqlist.insert( freqlist.end(), myLoader.m_freqs_eibi.begin(), myLoader.m_freqs_eibi.end() );
		readFreqSortAndDedupl();
	} else {
		/// No need to sort again the frequency list because we only remove elements.
		remove_source( qrg_mode_t::FREQS_EIBI );
	}
	updateSelect();
}
catch( const std::exception & exc )
{
	LOG_WARN("Caught %s when loading EIBI file:%s", exc.what(), LoaderEIBI::InstCatalog().Url() );
}

/// This loads the MWList stations file, about 11000 records.
struct LoaderMWList : public RecordLoader< LoaderMWList >
{
	FreqListType m_freqs_mwlist;

	/// Needed because we cannot built a nice filename from the URL.
	std::string base_filename() const {
		return "mwlist.txt";
	}

	void Clear() {
		m_freqs_mwlist.clear();
	}

	/// No intermediate storage.
	bool ReadRecord( std::istream & istrm ) {
		RecordMWList tmp ;
		istrm >> tmp ;
		if( istrm || istrm.eof() ) {
			m_freqs_mwlist.push_back( tmp );
			return true ;
		}
		return false ;
	}

	/// This file is also installed in data directory. The file is zipped.
	const char * Url(void) const {
		return "http://www.mwlist.org/mw_server.php?task=015&token=fldigi1803";
	}

	const char * Description(void) const {
		return _("MWList radio db");
	}
};

/// Updates the frequencies which are visible or not, depending on the flag.
void cb_MWList( Fl_Widget *, void *)
try
{
	/// Depending on this flag, frequencies loaded from a fixed file are visible or not.
	static bool flagMWList = false ;

	flagMWList = ! flagMWList ;

	LOG_INFO("flagMWList=%d", (int)flagMWList );

	LoaderMWList & myLoader = LoaderMWList::InstCatalog();

	static bool triedLoadMWList = false ;
	if( flagMWList ) {
		if( triedLoadMWList == false ) {
			triedLoadMWList = true ;
			int nbRec = myLoader.LoadAndRegister();
			if(nbRec < 0) {
				LOG_WARN("Error loading %s", myLoader.Url() );
				return ;
			}
		}
		LOG_INFO("Adding %d elements to %d", (int)myLoader.m_freqs_mwlist.size(), (int)freqlist.size() );
		freqlist.insert( freqlist.end(), myLoader.m_freqs_mwlist.begin(), myLoader.m_freqs_mwlist.end() );
		readFreqSortAndDedupl();
	} else {
		/// No need to sort again the frequency list because we only remove elements.
		remove_source( qrg_mode_t::FREQS_MWLIST );
	}
	updateSelect();
}
catch( const std::exception & exc )
{
	LOG_WARN("Caught %s when loading MWList file:%s", exc.what(), LoaderMWList::InstCatalog().Url() );
}

int cb_qso_opBW()
{
	if (progdefaults.chkUSERIGCATis)
//    if (!progdefaults.chkUSEXMLRPCis)
		rigCAT_setwidth(qso_opBW->value());
	return 0;
}

static void sendFreq(long long f)
{
#if USE_HAMLIB
	if (progdefaults.chkUSEHAMLIBis)
		hamlib_setfreq(f);
	else
#endif
		rigCAT_setfreq(f);
}

/// Callback of the frequency widget.
void qso_movFreq(Fl_Widget* w, void*)
{
	cFreqControl *fc = (cFreqControl *)w;
	long long f;
	f = fc->value();
	if (fc == qsoFreqDisp1) {
		qsoFreqDisp2->value(f);
		qsoFreqDisp3->value(f);
	} else if (fc == qsoFreqDisp2) {
		qsoFreqDisp1->value(f);
		qsoFreqDisp3->value(f);
	} else {
		qsoFreqDisp1->value(f);
		qsoFreqDisp2->value(f);
	}

	qso_displayFreq(f);

	sendFreq(f);
	return;
}

/// Adjusts the frequency list to the nearest frequency of the current one.
void qso_displayFreq( long long freq )
{
	/// The others members are defaulted.
        qrg_mode_t m(freq);

	int nbfreq = freqlist.size();
	if( nbfreq == 0 ) return ;

	/// The frequencies are sorted so no need to iterate.
        FreqListType::const_iterator pos = lower_bound(freqlist.begin(), freqlist.end(), m);

	if( pos != freqlist.end() )
		while( ( pos != freqlist.begin() ) && ( freq < pos->rfcarrier - pos->carrier ) ) --pos ;

	int idx = -1 ;
	bool frq_in_rng = false ;
        if (pos == freqlist.end()) {
		frq_in_rng = false ;
		idx = nbfreq - 1 ;
	} else if(
			( freq >= pos->rfcarrier - pos->carrier )
		&&	( freq <= pos->rfcarrier - pos->carrier + IMAGE_WIDTH ) )
	{
		/// If we are right in the range.
		frq_in_rng = true ;
		idx = pos - freqlist.begin();
	} else {
		frq_in_rng = false ;
		if (pos == freqlist.begin())
			idx = -1 ;
		else
			idx = pos - freqlist.begin();
	}

	if( ( idx >= 0 ) && ( idx < nbfreq ) ) {
		// int old_idx = qso_opBrowser->value();
		qso_opBrowser->deselect();
		if( ( nbfreq != qso_opBrowser->size() ) || ( idx >= nbfreq ) ) {
			throw std::runtime_error("Inconsistency");
		}
		/// Indices start at 1.
		++idx ;
		if( frq_in_rng ) {
			/// TODO: Select all lines which have this frequency.
			qso_opBrowser->select(idx);
		}

		qso_opBrowser->middleline(idx);
	}
	// long long curr_carrier = freqlist[n].rfcarrier + freqlist[n].carrier ;
}

void qso_selectFreq()
{
	int n = qso_opBrowser->value();
	if (!n) return;

	n -= 1;
// transceiver frequency
	if (freqlist[n].rfcarrier > 0) {
		qsoFreqDisp1->value(freqlist[n].rfcarrier);
		qsoFreqDisp2->value(freqlist[n].rfcarrier);
		qsoFreqDisp3->value(freqlist[n].rfcarrier);
		sendFreq(freqlist[n].rfcarrier);
	}
// transceiver mode
	if (freqlist[n].rmode != "NONE") {
		qso_opMODE->value(freqlist[n].rmode.c_str());
		cb_qso_opMODE();
	}
// modem type & audio sub carrier
	if (freqlist[n].mode != NUM_MODES) {
		if (freqlist[n].mode != active_modem->get_mode())
			init_modem_sync(freqlist[n].mode);
		if (freqlist[n].carrier > 0)
			active_modem->set_freq(freqlist[n].carrier);
	}

	/// Some details of the selected frequency are copied in the area used for logging.
	const char * tmpNam = freqlist[n].name.c_str();
	if( tmpNam && ( tmpNam[0] != '\0' ) ) inpCall1->value(tmpNam);
	const char * tmpDsc = freqlist[n].description.c_str();
	if( tmpDsc && ( tmpDsc[0] != '\0' ) ) inpName1->value(tmpDsc);
}

void qso_setFreq()
{
	int n = qso_opBrowser->value();
	if (!n) return;

	n -= 1;
// transceiver frequency
	if (freqlist[n].rfcarrier > 0) {
		qsoFreqDisp->value(freqlist[n].rfcarrier);
		sendFreq(freqlist[n].rfcarrier);
	}
}

void qso_delFreq()
{
	int v = qso_opBrowser->value() - 1;

	if (v >= 0) {
		freqlist.erase(freqlist.begin() + v);
		qso_opBrowser->remove(v + 1);
	}
}

void qso_addFreq()
{
	long freq = qsoFreqDisp->value();

	/// TODO: Remove duplicates with same frequency and mode.
	if (freq) {
		size_t pos = addtoList(freq);
		qso_opBrowser->insert(pos+1, freqlist[pos].str().c_str());
	}
}

void setTitle()
{
	if (windowTitle.length() > 0) {
		txtRigName->label(windowTitle.c_str());
		txtRigName->redraw_label();
	} else {
		txtRigName->label();
		txtRigName->redraw_label();
	}
}

bool init_Xml_RigDialog()
{
	LOG_DEBUG("xml rig");
	initOptionMenus();
	clearList();
	buildlist();
	windowTitle = xmlrig.rigTitle;
	setTitle();
	return true;
}

bool init_NoRig_RigDialog()
{
	LOG_DEBUG("no rig");
	qso_opBW->deactivate();
	qso_opMODE->clear();

	for (size_t i = 0; i < sizeof(modes)/sizeof(modes[0]); i++) {
		qso_opMODE->add(modes[i].name);
	}
	LSBmodes.clear();
	LSBmodes.push_back("LSB");
	LSBmodes.push_back("CWR");
	LSBmodes.push_back("RTTY");
	LSBmodes.push_back("PKTLSB");

	qso_opMODE->index(0);
	qso_opMODE->activate();

	clearList();
	buildlist();

	windowTitle = _("Enter Xcvr Freq");
	setTitle();

	return true;
}

#if USE_HAMLIB
bool init_Hamlib_RigDialog()
{
	LOG_DEBUG("hamlib");

	qso_opBW->deactivate();
	qso_opMODE->clear();

	for (size_t i = 0; i < sizeof(modes)/sizeof(modes[0]); i++) {
		mode_nums[modes[i].name] = modes[i].mode;
		mode_names[modes[i].mode] = modes[i].name;
		qso_opMODE->add(modes[i].name);
	}
	clearList();
	buildlist();

	windowTitle = "Hamlib ";
	windowTitle.append(xcvr->getName());

	setTitle();

	return true;
}
#endif
