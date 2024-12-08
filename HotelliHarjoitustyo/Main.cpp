//Niilo Tarri
//Hotelli Harjoitustyˆ, 5p

#include <iostream> //input/output
#include <ctime> //mm. satuinnaisluvut, aika
#include <array> //lis‰‰ array toimintoja
#include <string> //mm. stoi komento
#include <fstream> //tiedostoon tallentaminen ja tiedostosta lukeminen
#include <algorithm> //transform komentoa varten
#include <iomanip> //setw komento k‰ytt‰‰ t‰t‰, saadaan siistimm‰t tulostukset
#include <math.h> //k‰ytet‰‰n mm. round() komentoa varten.
#include <chrono> //k‰ytet‰‰n ottamaan tietokoneen t‰m‰n hetkinen p‰iv‰m‰‰r‰
#include <sstream> //k‰ytet‰‰n FormatisoiPvm()

//sallivat eteenkin windows alustoilla ‰‰kkˆsten k‰ytˆn, kun k‰sitell‰‰n k‰ytt‰j‰n inputteja wstring muuttujiin.
#include <cwctype>
#include <locale>
#define NOMINMAX
#include <windows.h>

using namespace std;

const string varaukset_file = "varaukset";
const string varaukset_backup_file = "varaukset_backup";
const string varaukset_arkisto_file = "varaukset_arkisto";

const int nimiMax = 50; //nimen max merkit
const int nimiMin = 3; //nimen (string syˆtteen) minimi merkit
const int hintaYhh = 100; // yhden hengen huoneen yˆhinta
const int hintaKhh = 150; //kahden hengen huoneen yˆhinta
const int huoneMaara = 300; //huoneiden m‰‰r‰, ATM ensimm‰inen puolisko on yhden hengen huoneita ja j‰lkimm‰inen puolisko kahden hengen huoneita
const int maxVarausKesto = 500; //hatusta heitetty luku, koodi tukee pidempi‰kin varauksia.
const int maxVuosi = 10; //Kuinka monen vuoden p‰‰h‰n k‰ytt‰j‰ voi tehd‰ varauksen.

//voisi asettaa korkeammalle, mutta t‰llˆin varaukset ladatessa muistipiikki ja viive kasvaa, 89999 riitt‰nee, plus varausnumerot riitt‰v‰t parhaillaan vain 89999 varaukselle muutenkin.
//1000000 max varausta on n. 150MB muistipiikki luettaessa varauksia.
//voisi varmaankin korjata tavalla tai toisella dynaamisilla muuttujilla, mutta n‰in toimii t‰h‰n tarpeeseen t‰ydellisesti.
const int maxVaraukset = 89999;
const int minVarausnumero = 10000;
const int maxVarausnumero = 99999;

const int varausTietoMaara = 8; //t‰ytyy manuaalisesti vaihtaa m‰ts‰‰m‰‰n varaus structin muuttujien m‰‰r‰‰, k‰ytet‰‰n kun ladataan varauksia
struct varaus
{
	int varausnumero = 0;
	int huonenumero = 0;
	int yot = 0;
	wstring nimi = L"";
	double hinta = 0; 
	int alkaaP = 0; //alkamisp‰iv‰
	int alkaaK = 0; //alkamiskuukausi
	int alkaaV = 0; //alkamisvuosi
};



//Formatisoi varauksen tiedot standardisoidulla tavalla, mill‰ voidaan tallentaa suoraan tiedostoon
void FormatisoiOutput(wostream& out, int varausnumero, int huonenumero, int yot, wstring nimi, double hinta, int alkaaP, int alkaaK, int alkaaV)
{
	out << varausnumero << ";" << huonenumero << ";" << yot << ";" << nimi << ";" << hinta << ";" << alkaaP << ";" << alkaaK << ";" << alkaaV << ";\n";
}



//Palauttaa bool muuttujan joka vastaa kysymykseen: onko vuosi karkausvuosi?
bool isLeapYear(int year)
{
	return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}



//P‰‰ttelee kuinka monta p‰iv‰‰ kuukaudessa on, ottaa huomioon karkausvuodet.
int daysInMonth(int month, int year)
{
	switch (month) {
	case 1: case 3: case 5: case 7: case 8: case 10: case 12: //31 p‰iv‰‰
		return 31;
	case 4: case 6: case 9: case 11: //30 p‰iv‰‰
		return 30;
	case 2: //28 tai 29 p‰iv‰‰
		return isLeapYear(year) ? 29 : 28;
	default:
		return 0;
	}
}



//Lis‰‰ p‰iv‰m‰‰r‰‰n x m‰‰r‰n p‰ivi‰
void addDays(int& day, int& month, int& year, int daysToAdd)
{
	while (daysToAdd > 0) {
		//pit‰‰ tiet‰‰ kuinka monta p‰iv‰‰ kuukaudessa on
		int daysInCurrentMonth = daysInMonth(month, year);

		//jos p‰iv‰t riitt‰v‰t, lis‰t‰‰n p‰iv‰t
		if (day + daysToAdd <= daysInCurrentMonth)
		{
			day += daysToAdd;
			break;
		}
		//jos ei, lis‰t‰‰n kuukauteen 1 ja jos kuukausi menee yli 12, flipataan takaisin yhteen ja lis‰t‰‰n vuoteen yksi.
		else {
			daysToAdd -= (daysInCurrentMonth - day + 1);
			day = 1;
			if (++month > 12) {
				month = 1;
				++year;
			}
		}
	}
}



// Tarkistaa onko varaukset p‰‰llek‰in
bool VarauksetPaallekain(int d1_start, int m1_start, int y1_start, int x1,
	int d2_start, int m2_start, int y2_start, int x2) {

	// Lasketaan ensimm‰isen varauksen loppumispvm
	int d1_end = d1_start, m1_end = m1_start, y1_end = y1_start;
	addDays(d1_end, m1_end, y1_end, x1);

	// Lasketaan toisen varauksen loppumispvm
	int d2_end = d2_start, m2_end = m2_start, y2_end = y2_start;
	addDays(d2_end, m2_end, y2_end, x2);

	// Jos toinen varaus loppuu ennen kuin toinen alkaa (t‰m‰ tarkistetaan molemmin p‰in) varaukset eiv‰t ole p‰‰llek‰iset.
	if ((y1_end < y2_start) || (y1_end == y2_start && m1_end < m2_start) ||
		(y1_end == y2_start && m1_end == m2_start && d1_end < d2_start)) {
		return false; //Ei p‰‰llek‰isyytt‰
	}
	if ((y2_end < y1_start) || (y2_end == y1_start && m2_end < m1_start) ||
		(y2_end == y1_start && m2_end == m1_start && d2_end < d1_start)) {
		return false; //Ei p‰‰llek‰isyytt‰
	}
	return true; //P‰‰lek‰isyys havaittu
}



//K‰ytet‰‰n, kun haetaan nykyinen koneen pvm.
//Muokkaa parametrina annetut muuttujat year, month ja day p‰iv‰m‰‰r‰ksi, annetun epoch ajan mukaan.
//Ei k‰ytet‰ valmista library‰, sill‰ niiss‰ tuli vastaan thread-safety ja yhteensopivuus ongelmia.
void EpochToDate(std::chrono::seconds epoch_seconds, int& year, int& month, int& day) {
	// Epoch starts at January 1, 1970, so we calculate from there
	int days_since_epoch = static_cast<int>(epoch_seconds.count() / 86400); // 86400 seconds in a day

	// Calculate the year
	year = 1970;
	while (true) {
		// Leap year check: 366 days in a leap year
		int days_in_current_year = (isLeapYear(year)) ? 366 : 365;
		if (days_since_epoch < days_in_current_year) break;
		days_since_epoch -= days_in_current_year;
		year++;
	}

	// Calculate the month
	month = 1;
	int days_in_current_month = daysInMonth(month, year);
	while (days_since_epoch >= days_in_current_month) {
		days_since_epoch -= days_in_current_month;
		month++;
		days_in_current_month = daysInMonth(month, year);
	}

	// Remaining days correspond to the day of the month
	day = days_since_epoch + 1; // days_since_epoch is 0-indexed, so add 1
}



//Hakee tietokoneen t‰m‰n hetkisen p‰iv‰m‰‰r‰n ja tallentaa sen p‰iv‰n kuukauden ja vuoden parametrina annettuihin muuttujiin.
void HaeNykyPvm(int &paiva, int &kuukausi, int &vuosi)
{
	auto now = chrono::system_clock::now();
	auto epoch_seconds = chrono::duration_cast<chrono::seconds>(now.time_since_epoch());

	EpochToDate(epoch_seconds, vuosi, kuukausi, paiva);

	return;
}



//Formatisoi p‰iv‰m‰‰r‰n muotoon, mik‰ voidaan tulostaa k‰ytt‰j‰lle
wstring FormatisoiPvm(int alkaaP, int alkaaK, int alkaaV)
{
	wstringstream wss;

	int luvut[] = { alkaaP, alkaaK, alkaaV };

	for (size_t i = 0; i < sizeof(luvut) / sizeof(luvut[0]); ++i)
	{
		//laitetaan p‰iv‰, kuukausi ja vuosi streamiin ja tulostetaan delimiterill‰ "."
		wss << luvut[i];

		if (i != sizeof(luvut) / sizeof(luvut[0]) - 1) {
			wss << L".";
		}
	}

	return wss.str();
}



//Lataa varaukset tiedostosta varaukset ja siirt‰‰ ne parametrin‰ annettuun heap array muuttujaan.
//Kun k‰ytet‰‰n, on t‰rke‰‰ muistaa poistaa muuttuja k‰ytˆn j‰lkeen, sill‰ sille ei ole en‰‰ tarvetta ja ei halua antaa niiden kasaantua.
void LataaVaraukset(varaus* varaukset)
{
	wifstream input;
	input.open(varaukset_file);


	wstring line, tieto, delimiter = L";"; //delimiteri ';' erottaa varauksen tiedot toisistaan tiedostossa varaukset
	int monesVaraus = 0; //ladataan yksi varaus kerrallaan, yksi varaus per tiedoston rivi, pit‰‰ muistaa mones varaus menossa.
	if (input.is_open())
	{
		while (getline(input, line) && monesVaraus < maxVaraukset - 1) //jatketaan niin kauan kun rivej‰ riitt‰‰ tai osutaan maksimi varausm‰‰r‰‰n
		{
			for (int i = 0; i < varausTietoMaara; i++) //toistetaan niin monta kertaa kun yhdess‰ varauksessa on eri tietoja
			{
				tieto = line.substr(0, line.find(delimiter)); //ladataan tiedostosta yhdelt‰ rivilt‰ yksi tieto
				line.erase(0, line.find(delimiter) + delimiter.length()); //ja poistetaan se rivilt‰

				//i kertoo meille, mit‰ tietoa k‰sitell‰‰n
				switch (i)
				{
				case 0:
					varaukset[monesVaraus].varausnumero = stoi(tieto);
					break;
				case 1:
					varaukset[monesVaraus].huonenumero = stoi(tieto);
					break;
				case 2:
					varaukset[monesVaraus].yot = stoi(tieto);
					break;
				case 3:
					varaukset[monesVaraus].nimi = (tieto);
					break;
				case 4:
					varaukset[monesVaraus].hinta = stod(tieto);
					break;
				case 5:
					varaukset[monesVaraus].alkaaP = stoi(tieto);
					break;
				case 6:
					varaukset[monesVaraus].alkaaK = stoi(tieto);
					break;
				case 7:
					varaukset[monesVaraus].alkaaV = stoi(tieto);
					break;
				default:
					break;
				}
			}
			monesVaraus++;
		}
	}
	input.close();
}



//tarkistaa parametrina annetun wstring muuttujan
//palauttaa true jos sis‰lt‰‰ vain kirjaimia ja v‰lilyˆntej‰, muuten false.
bool OnVainKirjaimiaJaValilyonteja(wstring merkkijono)
{
	for (wchar_t c : merkkijono)
	{
		if (!std::iswalpha(c) && c != ' ' && c != '-')
		{
			return false;
		}
	}
	return true;
}



//K‰ytet‰‰n tarkistamaan, onko wstring muuttujassa vain numeroita, mm. KysyInt() varten.
bool OnVainNumeroita(wstring merkkijono)
{
	for (wchar_t ch : merkkijono)
	{
		if (!iswdigit(ch))
		{
			return false;
		}
	}
	return true;
}



//Palauttaa kokonaislukuna int muuttujan pituuden, k‰ytet‰‰n mallailemaan wcoutit n‰tisti :)
int getIntLength(int integer)
{
	return to_string(integer).length();
}



//trimmaa parametrina annetusta wstring muuttujasta alusta ja lopusta v‰lilyˆnnit pois. keskell‰ olevat v‰lilyˆnnit j‰‰v‰t paikoilleen
wstring Trim(wstring merkkijono)
{
	int start = merkkijono.find_first_not_of(' ');
	int end = merkkijono.find_last_not_of(' ');
	return merkkijono.substr(start, end - start + 1);
}



//palauttaa parametrina annetun wstring muuttujan t‰ysin pienille kirjaimille muutettuna
wstring ToLowerCase(wstring merkkijono)
{
	wstring nMerkkijono = merkkijono;
	transform(nMerkkijono.begin(), nMerkkijono.end(), nMerkkijono.begin(), ::towlower);
	return nMerkkijono;
}



//Kysyy k‰ytt‰j‰lt‰ integer syˆtteen, funktio osaa tarkistaa laittomat syˆtteet ja t‰llˆin pyyt‰‰ syˆtett‰ uudestaan.
//parametrina annetaan halutun integerin maksimiarvo
int KysyInt(int max)
{
	wstring syote;
	int syoteInt;
	bool syoteOnInt;
	bool validSyote = false;

	do
	{
		wcout << "Syote: ";

		wcin >> syote;

		//tarkistetaan onko syˆtteess‰ vain numeroita, jos ei, annetaan oma virhehuomautus ja annetaan k‰ytt‰j‰n yritt‰‰ uudelleen.

		if (OnVainNumeroita(syote))
		{
			syoteInt = stoi(syote);
			syoteOnInt = true;
		}
		else
		{
			syoteOnInt = false;
		}

		if (!syoteOnInt)
		{
			wcout << "\nEi sallittu syote: Ei kokonaisluku.\n";
		}
		else if (syoteOnInt && syoteInt >= 0 && syoteInt <= max)
		{
			validSyote = true;
		}
		else //syˆtteen integeri on joko liian suuri tai alle 0
		{
			wcout << "\nEi sallittu syote: Alle nolla tai liian suuri kokonaisluku.\n";
		}
	} while (!validSyote);
	return syoteInt;
}



//Hoitaa nimen syˆtteen kysymisen ja sen validoimisen, parametrin‰ integerin‰ halutun wstringin minimi ja maksimi pituus
//Ei anna ohjeita k‰ytt‰j‰lle.
wstring KysyNimi(int min, int max)
{
	wstring syote;
	bool validSyote = false;

	//jos ei ignoorata edellist‰ inputtia, ohjelma antaa heti virhekoodin ennen kuin k‰ytt‰j‰ kerke‰‰ tehd‰ omaa syˆtett‰‰n.
	//oletettavasti koska getline komento lukee inputin historiasta yhden rivin.
	wcin.ignore(numeric_limits<streamsize>::max(), '\n');

	do
	{
		wcout << "Syote: ";

		getline(wcin, syote);

		syote = Trim(syote);

		if (syote == L"0") //jos syˆte on 0, k‰ytt‰j‰ haluaa peruuttaa nykyisen toiminnan
		{
			validSyote = true;
		}
		else if (!OnVainKirjaimiaJaValilyonteja(syote))
		{
			wcout << "Syotteessa ei ole vain kirjaimia, valilyontej‰ ja v‰liviivoja.\n";
		}
		else if (syote.length() > max)
		{
			wcout << "Maksimi syˆtteen pituus " << max << " merkki‰.\n";
		}
		else if (syote.length() < min)
		{
			wcout << "Minimi syˆtteen pituus " << min << " merkki‰.\n";
		}
		else
		{
			validSyote = true;
		}
	} while (!validSyote);
	return syote;
}



//Kysyy k‰ytt‰j‰lt‰ p‰iv‰m‰‰r‰n. Kykenee h‰ndl‰‰m‰‰n laittomat syˆtteet mukaanlukien:
//menneisyyden p‰iv‰m‰‰r‰t, liian iso p‰iv‰arvo (osaa reagoida karkausvuosiin), teksti‰ syˆtteess‰, liian iso kuukausiarvo
void KysyPvm(int& alkaaP, int& alkaaK, int& alkaaV)
{
	bool syotteetOnInt = false;

	wstring syoteStrP, syoteStrK, syoteStrV;

	int syoteP, syoteK, syoteV;

	int nykyP, nykyK, nykyV;

	HaeNykyPvm(nykyP, nykyK, nykyV);

	bool pvmOk = false;

	while (!pvmOk)
	{
		cin.ignore(numeric_limits<streamsize>::max(), '\n'); //ilman t‰t‰ paska osuu usein tuulettimeen, ilman t‰h‰: jos annettu mm. liikaa syˆtteit‰ aiemmin, se laskisi ne jo p‰iviksi ja kuukausiksi ja k‰ytt‰j‰n syˆtt‰ess‰ 3 lukua, kierre jatkuisi.
		wcout << "Syˆte: ";
		wcin >> syoteStrP >> syoteStrK >> syoteStrV;

		if (OnVainNumeroita(syoteStrP) && OnVainNumeroita(syoteStrK) && OnVainNumeroita(syoteStrV))
		{
			syoteP = stoi(syoteStrP);
			syoteK = stoi(syoteStrK);
			syoteV = stoi(syoteStrV);
			syotteetOnInt = true;
		}
		else
		{
			wcout << "Yksi tai useampi syˆte ei ole vain numeroita. Erotathan p‰iv‰t, kuukaudet ja vuodet v‰lilyˆnneill‰.\n";
			syotteetOnInt = false;
		}

		if (syotteetOnInt)
		{
			if (syoteP == 0 && syoteK == 0 && syoteV == 0) //jos syˆtteet on kaikki 0, k‰ytt‰j‰ haluaa peruuttaa.
			{
				alkaaP = 0;
				alkaaK = 0;
				alkaaV = 0;
				return;
			}

			if (syoteV > nykyV + maxVuosi)
			{
				wcout << "Vuosi tulee olla maksimissaan " << maxVuosi << " vuotta tulevaisuudessa.\n";
			}
			else if (syoteK > 12 || syoteK < 1)
			{
				wcout << "Kuukausi t‰ytyy olla v‰lilt‰ 1-12.\n";
			}
			else if (syoteP < 1)
			{
				wcout << "P‰iv‰ t‰ytyy olla v‰hint‰‰n 1.\n";
			}
			else if (syoteP > daysInMonth(syoteK, syoteV))
			{
				wcout << "Syˆtt‰m‰ss‰si kuukaudessa " << syoteK << " ei ole noin montaa p‰iv‰‰.\n";
			}
			else if (syoteV < nykyV)
			{
				wcout << "Et voi asettaa varausta menneisyyteen.\n";
			}
			else if (syoteV == nykyV && syoteK < nykyK)
			{
				wcout << "Et voi asettaa varausta menneisyyteen.\n";
			}
			else if (syoteV == nykyV && syoteK == nykyK && syoteP < nykyP)
			{
				wcout << "Et voi asettaa varausta menneisyyteen.\n";
			}
			else
			{
				alkaaP = syoteP;
				alkaaK = syoteK;
				alkaaV = syoteV;
				pvmOk = true;
			}
		}
	}
	return;
}



//palauttaa huoneMaara mittaisen array muuttujan, miss‰ jokaisella tiedolla on joko true tai false
//arrayn index kertoo huonenumeron ja true meinaa varattua huonetta
array<bool, huoneMaara> HaeVaratutHuoneet(int alkaaP, int alkaaK, int alkaaV, int kesto)
{
	varaus* varaukset = new varaus[maxVaraukset];

	LataaVaraukset(varaukset);

	array<bool, huoneMaara> varatutHuoneet;

	for (bool& huone : varatutHuoneet)
	{
		huone = false;
	}

	for (int i = 0;i < maxVaraukset;i++) //tsekataan kaikki varaukset l‰pi
	{
		if (varaukset[i].huonenumero != 0) //jos varaus on p‰tev‰
		{
			if (VarauksetPaallekain(varaukset[i].alkaaP, varaukset[i].alkaaK, varaukset[i].alkaaV, varaukset[i].yot,
				alkaaP, alkaaK, alkaaV, kesto)) //Jos VarauksetPaallekkain() palauttaa true, huone on varattu kys. p‰iv‰n‰.
			{
				varatutHuoneet[varaukset[i].huonenumero - 1] = true;
			}
		}
	}

	delete[] varaukset;

	return varatutHuoneet;
}



//Tarkistaa parametrina annetun varausnumeron ja palauttaa true jos se on vapaana ja false jos se on jo k‰ytˆss‰.
bool TarkistaVarausnumero(int varausnumero)
{
	varaus* varaukset = new varaus[maxVaraukset];

	LataaVaraukset(varaukset);

	if (varausnumero < minVarausnumero || varausnumero > maxVarausnumero)
	{
		delete[] varaukset;
		return false;
	}

	for (int i = 0;i < maxVaraukset; i++)
	{
		if (varaukset[i].varausnumero == varausnumero)
		{
			delete[] varaukset;
			return false;
		}
	}

	//lopuksi t‰ytyy poistaa heapista varaukset muuttuja ettei j‰‰ roikkumaan.
	delete[] varaukset;
	return true;
}



//Valitsee parametrein‰ annetusta varatutHuoneet arraysta ja huonetyypist‰ p‰‰tellen asiakkaalle huoneen satunnaisesti.
int ValitseHuone(array<bool, huoneMaara> varatutHuoneet, bool khh)
{
	srand(time(NULL));

	bool huoneOk = false;

	int huone;
	if (khh)
	{
		do
		{
			huone = (rand() % huoneMaara / 2 + 1) + huoneMaara / 2;
		} while (varatutHuoneet[huone - 1]); //jos huone on jo varattu, toista, halutaan huone mik‰ ei ole varattu
	}
	else
	{
		do
		{
			huone = rand() % huoneMaara / 2 + 1;
		} while (varatutHuoneet[huone - 1]); //jos huone on jo varattu, toista, halutaan huone mik‰ ei ole varattu
	}
	return huone;
}



//Palauttaa true jos huone on vapaana ja false jos huone on varattuna.
//Palauttaa myˆs false jos huonetyyppi ei matchaa
bool ValidoiHuone(array<bool, huoneMaara> varatutHuoneet, int huone, bool khh)
{


	if (khh && huone <= huoneMaara / 2)
	{
		wcout << "\nHuone ei ole kahden hengen huone, jos haluat kuitenkin yhden hengen huoneen => Syota 0 ja luo uusi varaus\n";
		return false;
	}

	if (!khh && huone > huoneMaara / 2)
	{
		wcout << "\nHuone ei ole yhden hengen huone, jos haluat kuitenkin kahden hengen huoneen => Syota 0 ja luo uusi varaus\n";
		return false;
	}

	if (varatutHuoneet[huone - 1])
	{
		wcout << "\nHuone ei ole vapaa aikaisemmin syˆtettyn‰ p‰iv‰n‰.\n";
	}

	return (!varatutHuoneet[huone - 1]); //palautetaan varattujen huoneiden vastakohta, true jos varattu=false jne.

}



//Kysyy k‰ytt‰j‰lt‰ mink‰laisen huoneen h‰n haluaa tai halutessa valitsee huoneen k‰ytt‰j‰n puolesta.
//Hoitaa myˆs huoneen vapauden tarkistuksen.
//Muokkaa annettuihin parametreihin uudet k‰ytt‰j‰n syˆtt‰m‰t ja ohjelman validoimat arvot.
bool KysyHuone(int &huonenumero, int &alkaaP, int &alkaaK, int &alkaaV, int &kesto)
{
	int syote = 0;
	bool khh = false, huoneOk = false;

	//kysyt‰‰n huonetyyppi
	wcout << "\nHaluatko yhden vai kahden hengen huoneen?\n1 - Yhden hengen huone\n2 - Kahden hengen huone\n0 - Peruuta, takaisin p‰‰valikkoon\n";
	syote = KysyInt(2);
	if (syote == 0)
	{
		return false;
	}
	else if (syote == 2)
	{
		khh = true;
	}

	//kysyt‰‰n pvm
	wcout << "\nIlmoita p‰iv‰m‰‰r‰, milloin haluat varauksen alkavan muodossa (pp kk vvvv)\n0 0 0 - Peruuta, takaisin p‰‰valikkoon\n";
	KysyPvm(alkaaP, alkaaK, alkaaV);
	if (alkaaP == 0)
	{
		return false;
	}

	//kysyt‰‰n yˆt
	wcout << "\nKuinka monta yˆt‰ haluat viipya? Syˆt‰ ˆiden m‰‰r‰ kokonaislukuna. Maksimi ˆiden m‰‰r‰ on 3650. Tai:\n0 - Peruuta, takaisin p‰‰valikkoon\n";
	kesto = KysyInt(3650);
	if (kesto == 0)
	{
		return false;
	}

	//haetaan varatut huoneet
	array<bool, huoneMaara> varatutHuoneet = HaeVaratutHuoneet(alkaaP, alkaaK, alkaaV, kesto);

	bool vahintaanYksiHuoneVapaana = false;
	for (int i = 0; i < huoneMaara; i++)
	{
		if (!varatutHuoneet[i] || i + 1 == huonenumero) //jos huone on vapaana tai tarkistettava huone on sama kuin k‰ytt‰j‰n jo valitsema huone(muokatessa varausta) merkit‰‰n v‰hint‰‰n yksi huone vapaaksi
		{
			vahintaanYksiHuoneVapaana = true;
		}
	}

	if (!vahintaanYksiHuoneVapaana) //jos yksik‰‰n huone ei ole vapaana, varausta ei ole mahdollista luoda. Siirret‰‰n k‰ytt‰j‰ p‰‰valikkoon. luomatta/muokkaamatta varausta.
	{
		wcout << "\nValitettavasti kyseiselle aikajaksolle ei lˆyty vapaita";
		if (khh)
		{
			wcout << " kahden hengen huoneita. Teid‰t siirret‰‰n nyt takaisin p‰‰valikkoon.\n";
		}
		else
		{
			wcout << " yhden hengen huoneita. Teid‰t siirret‰‰n nyt takaisin p‰‰valikkoon.\n";
		}
		huonenumero = 0;
		alkaaP = 0;
		alkaaK = 0;
		alkaaV = 0;
		kesto = 0;
		return false;
	}

	do
	{
		if (khh)
		{
			wcout << "\nVapaat kahden hengen huoneet:\n";

			for (int i = huoneMaara / 2; i < huoneMaara; i++)
			{
				if (!varatutHuoneet[i] || i + 1 == huonenumero)
				{
					wcout << i + 1 << ", ";
				}
			}
			wcout << "\n";
		}
		else
		{
			wcout << "\nVapaat yhden hengen huoneet:\n";

			for (int i = 0; i < huoneMaara / 2; i++)
			{
				if (!varatutHuoneet[i] || i + 1 == huonenumero)
				{
					wcout << i + 1 << ", ";
				}
			}
			wcout << "\n";
		}

		wcout << "\nSyota haluamasi huoneen numero, tai:\n" << huoneMaara + 1 << " - Ohjelma valitsee huoneen puolestasi\n0 - Peruuta, takaisin p‰‰valikkoon\n";
		syote = KysyInt(huoneMaara + 1);
		if (syote == 0)
		{
			return false;
		}
		else if (syote == huoneMaara + 1)
		{
			huonenumero = ValitseHuone(varatutHuoneet, khh);
			huoneOk = true;
		}
		else
		{
			if (ValidoiHuone(varatutHuoneet, syote, khh) || huonenumero == syote)
			{
				huonenumero = syote;
				huoneOk = true;
			}
		}

	} while (!huoneOk);

	return true;
}



//Palauttaa varauksen hinnan annetuilla parametreilla, yˆt, huonenumero ja alennusprosentti
int AnnaHinta(int yot, int huonenumero, int alennusprosentti)
{

	if (huonenumero > huoneMaara / 2)
	{
		return double(yot) * double(hintaKhh) - double(yot) * double(hintaKhh) / 100.0 * double(alennusprosentti);
	}
	else
	{
		return double(yot) * double(hintaYhh) - double(yot) * double(hintaYhh) / 100.0 * double(alennusprosentti);
	}

}



//Luo varaus, yksi ohjelman p‰‰funktioista.
//Aloittaa varauksen luomisen aliohjelman
void LuoVaraus()
{
	varaus uusiVaraus;

	if (!KysyHuone(uusiVaraus.huonenumero, uusiVaraus.alkaaP, uusiVaraus.alkaaK, uusiVaraus.alkaaV, uusiVaraus.yot))
	{
		wcout << "\nPOISTUTAAN PƒƒVALIKKOON LUOMATTA TILAUSTA\n";
		return;
	}

	//Kysyt‰‰n nimi
	wcout << "\nSyˆt‰ nimesi. Syota vain kirjaimia, v‰lilyˆntej‰ ja v‰liviivoja. Pituus " << nimiMin << " - " << nimiMax << " merkki‰.\n0 - Peruuta, takaisin p‰‰valikkoon.\n";
	uusiVaraus.nimi = KysyNimi(nimiMin, nimiMax);
	if (uusiVaraus.nimi == L"0")
	{
		wcout << "\nPOISTUTAAN PƒƒVALIKKOON LUOMATTA TILAUSTA\n";
		uusiVaraus.huonenumero = 0;
		uusiVaraus.yot = 0;
		return;
	}

	//randomisoidaan aleprosentti, lopputulos 0, 10 tai 20
	int alennusprosentti = (rand() % 30);
	alennusprosentti = (alennusprosentti / 10) * 10;

	//lasketaan hinta
	uusiVaraus.hinta = AnnaHinta(uusiVaraus.yot, uusiVaraus.huonenumero, alennusprosentti);

	//tulostetaan varauksen tiedot
	wcout << "\n==============================\nVarauksesi tiedot:\nHuonenumero - " << uusiVaraus.huonenumero;
	if (uusiVaraus.huonenumero > huoneMaara / 2)
	{
		wcout << " (kahden hengen huone)\n";
	}
	else
	{
		wcout << " (yhden hengen huone)\n";
	}
	wstring pvm = FormatisoiPvm(uusiVaraus.alkaaP, uusiVaraus.alkaaK, uusiVaraus.alkaaV);
	wcout << "Aloituspvm: " << pvm << "\n÷iden m‰‰r‰ - " << uusiVaraus.yot << "\nNimi - " << uusiVaraus.nimi << "\n------------------------------\nHinta - " << uusiVaraus.hinta << "\n------------------------------\n";
	wcout << "\nHaluatko viimeistella tilauksen?\n1 - Kylla\n0 - Peruuta, takaisin p‰‰valikkoon\n";

	//pyydet‰‰n varmistus
	int varmistus;
	varmistus = KysyInt(1);
	if (varmistus == 0)
	{
		wcout << "\nPOISTUTAAN PƒƒVALIKKOON LUOMATTA TILAUSTA\n";
		uusiVaraus.huonenumero = 0;
		uusiVaraus.yot = 0;
		uusiVaraus.nimi = L"";
		uusiVaraus.hinta = 0;
		return;
	}

	bool varausnumerook = false;

	//v‰liaikaiset muuttujat, varausnumeron m‰‰ritt‰mist‰ varten.
	int tempHi, tempLo, temp;
	while (!varausnumerook)
	{
		tempHi = rand() % 899 + 1;
		tempLo = rand() % 99 + 1;
		temp = (tempHi * 100 + tempLo);
		if (TarkistaVarausnumero(temp))
		{
			uusiVaraus.varausnumero = temp;
			varausnumerook = true;
		}
	}

	wofstream output;
	output.open(varaukset_file, ios::app); //t‰ll‰ output tyylill‰ "ios::app" voidaan append, eli lis‰t‰ teksti‰ vain tiedoston loppuun
	output << uusiVaraus.varausnumero << ";" << uusiVaraus.huonenumero << ";" << uusiVaraus.yot << ";" << uusiVaraus.nimi << ";" << uusiVaraus.hinta << ";" << uusiVaraus.alkaaP << ";" << uusiVaraus.alkaaK << ";" << uusiVaraus.alkaaV << ";\n";
	output.close();

	wcout << "\nVaraus luotu. Varausnumero: " << uusiVaraus.varausnumero << "\n";

	return;
}



//Tarkastele varaus, yksi ohjelman p‰‰funktioista.
//Aloittaa varauksen etsimisen aliohjelman
void TarkasteleVarauksia()
{
	//wstring syote ja int syoteInt kaytetaan tallentamaan k‰ytt‰j‰n syˆte.
	wstring syote;
	int syoteInt;

	varaus* varaukset = new varaus[maxVaraukset];

	LataaVaraukset(varaukset);

	bool uudestaan = true; //k‰ytet‰‰n, kun halutaan palata takaisin p‰‰valikkoon.
	while (uudestaan)
	{

		bool syoteOnInt = true;
		bool etsi = true; //t‰ss‰ muistetaan, etsittiinkˆ tietoja vai ei
		int loytyi = 0; //lasketaan kuinka monta lˆydettiin

		wcout << "\nSyˆt‰ tieto mill‰ haluat etsia tilauksia (varausnumero, huonenumero, nimi tai osa nimest‰).\n0 - Peruuta, takaisin p‰‰valikkoon\nSyote: ";
		wcin >> syote;

		syote = Trim(syote);

		//kokeillaan muuntaa wstring integeriksi, jos ei onnistu, otetaan koppi errorista, merkit‰‰n ett‰ muuttuja ei ole int ja jatketaan normaalisti eteenp‰in.
		try
		{
			syoteInt = stoi(syote);
		}
		catch (exception& err)
		{
			syoteOnInt = false;
		}


		if (syoteOnInt && syoteInt == 0)
		{
			etsi = false;
			uudestaan = false;
		}
		else if (syoteOnInt && syoteInt <= huoneMaara && syoteInt > 0) //HUONENUMERO
		{
			//tulostetaan lˆydetyt varaukset n‰tisti
			wcout << "\nEtsitaan varauksia huonenumerolla " << syoteInt << "...\n";
			int kolumniLeveys = 12;
			wcout << "\n" << left
				<< setw(kolumniLeveys) << "Varausnumero" << " |"
				<< setw(kolumniLeveys) << "Huonenumero" << " |"
				<< setw(kolumniLeveys) << "÷iden m‰‰r‰" << " |"
				<< setw(kolumniLeveys) << "Alkamispvm" << " |"
				<< setw(kolumniLeveys) << "Nimi" << "\n";
			for (int i = 0; i < maxVaraukset; i++)
			{
				if (varaukset[i].varausnumero != 0)
				{
					if (varaukset[i].huonenumero == syoteInt)
					{
						wstring pvm = FormatisoiPvm(varaukset[i].alkaaP, varaukset[i].alkaaK, varaukset[i].alkaaV);
						wcout << left
							<< setw(kolumniLeveys) << varaukset[i].varausnumero << " |"
							<< setw(kolumniLeveys) << varaukset[i].huonenumero << " |"
							<< setw(kolumniLeveys) << varaukset[i].yot << " |"
							<< setw(kolumniLeveys) << pvm << " |"
							<< setw(kolumniLeveys) << varaukset[i].nimi << "\n";
						loytyi++;
					}
				}
			}
		}
		else if (syoteOnInt && syoteInt >= minVarausnumero && syoteInt <= maxVarausnumero) //VARAUSNUMERO
		{
			//tulostetaan lˆydetyt varaukset n‰tisti
			wcout << "\nEtsitaan varauksia varausnumerolla " << syoteInt << "...\n";
			int kolumniLeveys = 12;
			wcout << "\n" << left
				<< setw(kolumniLeveys) << "Varausnumero" << " |"
				<< setw(kolumniLeveys) << "Huonenumero" << " |"
				<< setw(kolumniLeveys) << "÷iden m‰‰r‰" << " |"
				<< setw(kolumniLeveys) << "Alkamispvm" << " |"
				<< setw(kolumniLeveys) << "Nimi" << "\n";

			for (int i = 0; i < maxVaraukset; i++)
			{
				if (varaukset[i].varausnumero != 0)
				{
					if (varaukset[i].varausnumero == syoteInt)
					{
						wstring pvm = FormatisoiPvm(varaukset[i].alkaaP, varaukset[i].alkaaK, varaukset[i].alkaaV);
					wcout << left
						<< setw(kolumniLeveys) << varaukset[i].varausnumero << " |"
						<< setw(kolumniLeveys) << varaukset[i].huonenumero << " |"
						<< setw(kolumniLeveys) << varaukset[i].yot << " |"
						<< setw(kolumniLeveys) << pvm << " |"
						<< setw(kolumniLeveys) << varaukset[i].nimi << "\n";
						loytyi++;
					}
				}
			}
		}
		else if (!OnVainKirjaimiaJaValilyonteja(syote))
		{
			wcout << "\nSyˆte ei validi, ei voida etsia.\n";
			etsi = false;
		}
		else //NIMI
		{
			//tulostetaan lˆydetyt varaukset n‰tisti
			wcout << "\nEtsit‰‰n varauksia nimell‰ " << syote << "...\n";
			int kolumniLeveys = 12;
			wcout << "\n" << left
				<< setw(kolumniLeveys) << "Varausnumero" << " |"
				<< setw(kolumniLeveys) << "Huonenumero" << " |"
				<< setw(kolumniLeveys) << "÷iden m‰‰r‰" << " |"
				<< setw(kolumniLeveys) << "Alkamispvm" << " |"
				<< setw(kolumniLeveys) << "Nimi" << "\n";

			for (int i = 0; i < maxVaraukset; i++)
			{
				if (varaukset[i].varausnumero != 0)
				{
					if (ToLowerCase(varaukset[i].nimi).find(ToLowerCase(syote)) != varaukset[i].nimi.npos)
					{
						wstring pvm = FormatisoiPvm(varaukset[i].alkaaP, varaukset[i].alkaaK, varaukset[i].alkaaV);
						wcout << left
							<< setw(kolumniLeveys) << varaukset[i].varausnumero << " |"
							<< setw(kolumniLeveys) << varaukset[i].huonenumero << " |"
							<< setw(kolumniLeveys) << varaukset[i].yot << " |"
							<< setw(kolumniLeveys) << pvm << " |"
							<< setw(kolumniLeveys) << varaukset[i].nimi << "\n";
						loytyi++;
					}
				}
			}

		}

		if (etsi && loytyi == 0)
		{
			wcout << "\nHakuehdoilla ei lˆytynyt varauksia\n";
		}
		else if (etsi)
		{
			wcout << "\nHakuehdoilla lˆytyi " << loytyi << " varaus";
			if (loytyi > 1)
			{
				wcout << "ta\n";
			}
			else
			{
				wcout << "\n";
			}
		}

	}
	delete[] varaukset;
	return;
}



//Aliohjelma, peru varaus, kysyy k‰ytt‰j‰lt‰ mink‰ haluaa peruuttaa ja peruuttaa sen.
void PeruVaraus()
{
	int syote;
	bool loytyi = false;

	varaus* varaukset = new varaus[maxVaraukset];

	LataaVaraukset(varaukset);

	while (!loytyi)
	{

		wcout << "\nSyˆt‰ peruutettavan varauksen varausnumero\n0 - Takaisin p‰‰valikkoon\n";
		syote = KysyInt(maxVarausnumero);
		if (syote == 0)
		{
			delete[] varaukset;
			return;
		}

		if (syote > minVarausnumero && syote < maxVarausnumero)
		{
			for (int i = 0; i < maxVaraukset; i++)
			{
				if (varaukset[i].varausnumero == syote)
				{
					wstring pvm = FormatisoiPvm(varaukset[i].alkaaP, varaukset[i].alkaaK, varaukset[i].alkaaV);
					int kolumniLeveys = 12;
					wcout << "\n" << left
						<< setw(kolumniLeveys) << "Varausnumero" << " |"
						<< setw(kolumniLeveys) << "Huonenumero" << " |"
						<< setw(kolumniLeveys) << "÷iden m‰‰r‰" << " |"
						<< setw(kolumniLeveys) << "Alkamispvm" << " |"
						<< setw(kolumniLeveys) << "Nimi" << "\n";
					wcout << left
						<< setw(kolumniLeveys) << varaukset[i].varausnumero << " |"
						<< setw(kolumniLeveys) << varaukset[i].huonenumero << " |"
						<< setw(kolumniLeveys) << varaukset[i].yot << " |"
						<< setw(kolumniLeveys) << pvm << " |"
						<< setw(kolumniLeveys) << varaukset[i].nimi << "\n";
					loytyi = true;
				}
			}
			if (!loytyi)
			{
				wcout << "\nTilausta " << syote << " ei lˆytynyt.\n";
			}
		}
		else
		{
			wcout << "\nSyˆte '" << syote << "' ei ole validi.\n";
		}
	}


	if (loytyi)
	{
		int varmistus;
		wcout << "\nVaraus lˆytyi. Haluatko varmasti peruuttaa?\n1 - Kyll‰, peruuta varaus\n0 - Ei, takaisin p‰‰valikkoon\n";
		varmistus = KysyInt(1);
		if (varmistus == 1)
		{
			//jos peruutetaan varaus, kirjoitetaan k‰yt‰nnˆss‰ koko varaukset tiedosto uudestaan, ilman peruutettavaa varausta
			wofstream output;
			output.open(varaukset_file);
			for (int i = 0; i < maxVaraukset; i++)
			{
				if (varaukset[i].varausnumero != syote && varaukset[i].varausnumero != 0)
				{
					FormatisoiOutput(output, varaukset[i].varausnumero, varaukset[i].huonenumero, varaukset[i].yot, varaukset[i].nimi, varaukset[i].hinta, varaukset[i].alkaaP, varaukset[i].alkaaK, varaukset[i].alkaaV);
				}
			}
			cout << "\nVaraus " << syote << " peruutettu.\n";
			output.close();
		}
	}

	delete[] varaukset;
	return;

}



//Muokkaa olemassa olevaa varausta. Huonetyyppi‰ tai p‰ivien m‰‰r‰‰ vaihdattaessa uusi hinta lasketaan automaattisesti samalla alennusprosentilla, mill‰ varaus oli luotu.
void MuokkaaVarausta()
{
	int syoteVarausnumero;
	bool loytyi = false;
	bool muokattu = false;

	varaus* varaukset = new varaus[maxVaraukset];

	varaus muokattavaVaraus;

	LataaVaraukset(varaukset);

	while (!loytyi)
	{

		wcout << "\nSyˆt‰ muokattavan varauksen varausnumero\n0 - Peruuta, takaisin p‰‰valikkoon\n";
		syoteVarausnumero = KysyInt(maxVarausnumero);
		if (syoteVarausnumero == 0)
		{
			delete[] varaukset;
			return;
		}


		if (syoteVarausnumero > minVarausnumero && syoteVarausnumero < maxVarausnumero)
		{
			for (int i = 0; i < maxVaraukset; i++)
			{
				if (varaukset[i].varausnumero == syoteVarausnumero)
				{
					wstring pvm = FormatisoiPvm(varaukset[i].alkaaP, varaukset[i].alkaaK, varaukset[i].alkaaV);
					int kolumniLeveys = 12;
					wcout << "\n" << left
						<< setw(kolumniLeveys) << "Varausnumero" << " |"
						<< setw(kolumniLeveys) << "Huonenumero" << " |"
						<< setw(kolumniLeveys) << "÷iden m‰‰r‰" << " |"
						<< setw(kolumniLeveys) << "Alkamispvm" << " |"
						<< setw(kolumniLeveys) << "Nimi" << "\n";
					wcout << left
						<< setw(kolumniLeveys) << varaukset[i].varausnumero << " |"
						<< setw(kolumniLeveys) << varaukset[i].huonenumero << " |"
						<< setw(kolumniLeveys) << varaukset[i].yot << " |"
						<< setw(kolumniLeveys) << pvm << " |"
						<< setw(kolumniLeveys) << varaukset[i].nimi << "\n";
					loytyi = true;

					muokattavaVaraus = varaukset[i];

				}
			}
			if (!loytyi)
			{
				wcout << "\nTilausta " << syoteVarausnumero << " ei lˆytynyt.\n";
			}
		}
		else
		{
			wcout << "\nSyˆte '" << syoteVarausnumero << "' ei ole validi.\n";
		}
	}


	if (loytyi)
	{
		int varmistusSyote; //k‰ytet‰‰n ottamaan viel‰ viimeinen varmistus, haluaako k‰ytt‰j‰ muokata varausta
		bool varmistus = true;
		int syote; //syˆte, mit‰ tietoa k‰ytt‰j‰ haluaa muokata
		int vanhaYot = muokattavaVaraus.yot; //vanhojen tietojen mukainen yˆm‰‰r‰, k‰ytet‰‰n laskemaan alennusprosenttia
		int vanhaHuonenumero = muokattavaVaraus.huonenumero; //vanhojen tietojen mukainen huonenumero, k‰ytet‰‰n laskemaan alennusprosenttia
		wcout << "\nVaraus lˆytyi. Mit‰ tietoa haluat muokata?\n1 - Huonenumeroa, alkamis p‰iv‰m‰‰r‰‰ tai kestoa\n2 - Nimi\n0 - Peruuta, takaisin p‰‰valikkoon\n";
		syote = KysyInt(2);
		if (syote == 0)
		{
			delete[] varaukset;
			return;
		}

		switch (syote)
		{
		case 1:
			
			if (!KysyHuone(muokattavaVaraus.huonenumero, muokattavaVaraus.alkaaP, muokattavaVaraus.alkaaK, muokattavaVaraus.alkaaV, muokattavaVaraus.yot))
			{
				varmistus = false;
			}
			else
			{
				int alkHinta;
				if (vanhaHuonenumero > huoneMaara / 2)
				{
					wcout << "khh";
					alkHinta = vanhaYot * hintaKhh;
				}
				else
				{
					wcout << "yhh";
					alkHinta = vanhaYot * hintaYhh;
				}
				wcout << alkHinta;
				wcout << round((alkHinta - muokattavaVaraus.hinta) / alkHinta * 100);
				muokattavaVaraus.hinta = AnnaHinta(muokattavaVaraus.yot, muokattavaVaraus.huonenumero, round((alkHinta - muokattavaVaraus.hinta) / alkHinta * 100)); //round(...) laskee alennusprosentin
			}
			break;
		case 2:
			wcout << "\nSyˆt‰ nimesi. Syota vain kirjaimia, v‰lilyˆntej‰ ja v‰liviivoja. Pituus " << nimiMin << " - " << nimiMax << " merkki‰.\n0 - Peruuta, takaisin p‰‰valikkoon.\n";
			muokattavaVaraus.nimi = KysyNimi(nimiMin, nimiMax);
			if (muokattavaVaraus.nimi == L"0")
			{
				varmistus = false;
			}
			break;
		default:
			delete[] varaukset;
			return;
			break;
		}

		if (varmistus)
		{
			wcout << "\n==============================\nVarauksesi tiedot:\nHuonenumero - " << muokattavaVaraus.huonenumero;

			if (muokattavaVaraus.huonenumero > huoneMaara / 2)
			{
				wcout << " (kahden hengen huone)\n";
			}
			else
			{
				wcout << " (yhden hengen huone)\n";
			}

			wstring pvm = FormatisoiPvm(muokattavaVaraus.alkaaP, muokattavaVaraus.alkaaK, muokattavaVaraus.alkaaV);

			wcout << "÷iden m‰‰r‰ - " << muokattavaVaraus.yot << "\nNimi - " << muokattavaVaraus.nimi << "\nAlkamispvm - " << pvm << "\n------------------------------\nHinta - " << muokattavaVaraus.hinta << "\n------------------------------\n";


			wcout << "\nHaluatko varmasti muokata varausta?\n1 - Kyll‰, muokkaa\n0 - Peruuta, takaisin p‰‰valikkoon\n";
			varmistusSyote = KysyInt(1);
			if (varmistusSyote == 0)
			{
				varmistus = false;
			}
			else
			{
				varmistus = true;
			}
		}

		if (varmistus)
		{
			//kun muokataan, kirjoitetaan koko tiedosto uudestaan mutta varausnumeroa m‰ts‰‰v‰ varaus kirjoitetaan uusilla tiedoilla.
			wofstream output;
			output.open(varaukset_file);
			for (int i = 0; i < maxVaraukset; i++)
			{
				if (varaukset[i].varausnumero != syoteVarausnumero && varaukset[i].varausnumero != 0)
				{
					FormatisoiOutput(output, varaukset[i].varausnumero, varaukset[i].huonenumero, varaukset[i].yot, varaukset[i].nimi, varaukset[i].hinta, varaukset[i].alkaaP, varaukset[i].alkaaK, varaukset[i].alkaaV);
				}
				else if (varaukset[i].varausnumero == syoteVarausnumero)
				{
					FormatisoiOutput(output, muokattavaVaraus.varausnumero, muokattavaVaraus.huonenumero, muokattavaVaraus.yot, muokattavaVaraus.nimi, muokattavaVaraus.hinta, muokattavaVaraus.alkaaP, muokattavaVaraus.alkaaK, muokattavaVaraus.alkaaV);

					muokattu = true;
				}
			}
			output.close();
		}
		if (muokattu)
		{
			cout << "\nVaraus " << syoteVarausnumero << " muokattu.\n";
		}
		else
		{
			cout << "\nVarausta ei muokattu.\n";
		}
	}

	delete[] varaukset;
	return;

}



//Kopioi varaukset tiedoston varaukset_backup tiedostoon
void TeeBackup()
{
	wcout << "\nTehd‰‰n varmuuskopiota varauksista...";

	wifstream input;
	input.open(varaukset_file);

	wofstream output;
	output.open(varaukset_backup_file);


	wstring line, tieto, delimiter = L";"; //delimiteri ';' erottaa varauksen tiedot toisistaan tiedostossa varaukset
	if (input.is_open())
	{
		while (getline(input, line)) //jatketaan niin kauan kun rivej‰ riitt‰‰
		{
			output << line << "\n"; //kopioidaan inputin rivi output tiedostoon
		}
	}
	input.close();
	output.close();
	
	wcout << "\nVarmuuskopiointi valmis 'varaukset_backup' -tiedostoon.\n";
	return;
}



//Poistaa aktiiviselta varauslistalta loppuneet varaukset ja arkistoi ne toiseen tiedostoon.
void ArkistoiLoppuneetVaraukset()
{
	array<int, huoneMaara> loppuneet; //t‰h‰n tallennetaan loppuneiden varausten varausnumerot maksimissaan huonem‰‰r‰n verran

	int montaPoistetaan = 0;

	varaus* varaukset = new varaus[maxVaraukset];

	LataaVaraukset(varaukset);

	int nykyP, nykyK, nykyV;

	int loppuuP, loppuuK, loppuuV;

	HaeNykyPvm(nykyP, nykyK, nykyV);

	for (int i = 0; i < maxVaraukset; i++)
	{
		if (varaukset[i].varausnumero != 0)
		{
			loppuuP = varaukset[i].alkaaP;
			loppuuK = varaukset[i].alkaaK;
			loppuuV = varaukset[i].alkaaV;
			addDays(loppuuP, loppuuK, loppuuV, varaukset[i].yot);


			if (nykyV > loppuuV)
			{
				loppuneet[montaPoistetaan] = varaukset[i].varausnumero;
				montaPoistetaan++;
			}
			else if (nykyV == loppuuV && nykyK > loppuuK)
			{
				loppuneet[montaPoistetaan] = varaukset[i].varausnumero;
				montaPoistetaan++;
			}
			else if (nykyV == loppuuV && nykyK == loppuuK && nykyP > loppuuP)
			{
				loppuneet[montaPoistetaan] = varaukset[i].varausnumero;
				montaPoistetaan++;
			}
		}
	}

	if (montaPoistetaan > 0)
	{
		wcout << "\nArkistoidaan " << montaPoistetaan << " kpl loppuneita varauksia.\n";
	}


	wofstream output, output_arkisto;
	output_arkisto.open(varaukset_arkisto_file, ios::app);
	output.open(varaukset_file);
	for (int i = 0; i < maxVaraukset; i++)
	{
		if (varaukset[i].varausnumero != 0)
		{
			if (find(begin(loppuneet), end(loppuneet), varaukset[i].varausnumero) == end(loppuneet))
			{
				FormatisoiOutput(output, varaukset[i].varausnumero, varaukset[i].huonenumero, varaukset[i].yot, varaukset[i].nimi, varaukset[i].hinta, varaukset[i].alkaaP, varaukset[i].alkaaK, varaukset[i].alkaaV);
			}
			else
			{
				//kun arkistoidaan, poistetaan varausnumero, sille ei ole en‰‰ tarvetta.
				FormatisoiOutput(output_arkisto, 0, varaukset[i].huonenumero, varaukset[i].yot, varaukset[i].nimi, varaukset[i].hinta, varaukset[i].alkaaP, varaukset[i].alkaaK, varaukset[i].alkaaV);
			}
		}
	}
	output.close();

	delete[] varaukset;
	return; 
}



//P‰‰ohjelmalooppi
int main()
{
	//seuraavat sallivat koodin k‰ytt‰‰ ‰‰kkˆsi‰ mm. wcout, wcin, wstring jne. muodossa
	setlocale(LC_ALL, "en_US.UTF-8");
	SetConsoleCP(CP_UTF8);    // Input code page (for wcin)
	SetConsoleOutputCP(CP_UTF8);  // Output code page (for wcout)

	srand(time(NULL));

	ArkistoiLoppuneetVaraukset();

	bool suljeOhjelma = false;;
	int syote;
	while (!suljeOhjelma)
	{
		wcout << "\n==============================\nPƒƒVALIKKO\n==============================\n";
		wcout << "Mit‰ haluat tehd‰?\n1 - Luo varaus\n2 - Etsi ja tarkastele varauksia\n3 - Muokkaa varausta\n4 - Peru varaus\n0 - Sulje ohjelma\n";
		syote = KysyInt(4);
		switch (syote)
		{
		case 1:
			LuoVaraus();
			break;
		case 2:
			TarkasteleVarauksia();
			break;
		case 3:
			MuokkaaVarausta();
			break;
		case 4:
			PeruVaraus();
			break;
		case 0:
			suljeOhjelma = true;
			break;
		default:
			break;
		}
	}

	TeeBackup();
	wcout << "\nSujetaan sovellus...\n";
	return 1;
}

