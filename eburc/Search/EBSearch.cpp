#include "pch.h"
#include "eburc/Search/EBSearch.h"

using namespace libeburc;

EBSearch::EBSearch()
{
	index_id = 0;
    start_page = 0;
    end_page = 0;
    candidates_page = 0;
    katakana = EBIndexStyleCode::EB_INDEX_STYLE_CONVERT;
    lower = EBIndexStyleCode::EB_INDEX_STYLE_CONVERT;
    mark = EBIndexStyleCode::EB_INDEX_STYLE_DELETE;
    long_vowel = EBIndexStyleCode::EB_INDEX_STYLE_CONVERT;
    double_consonant = EBIndexStyleCode::EB_INDEX_STYLE_CONVERT;
    contracted_sound = EBIndexStyleCode::EB_INDEX_STYLE_CONVERT;
    voiced_consonant = EBIndexStyleCode::EB_INDEX_STYLE_CONVERT;
    small_vowel = EBIndexStyleCode::EB_INDEX_STYLE_CONVERT;
    p_sound = EBIndexStyleCode::EB_INDEX_STYLE_CONVERT;
    space = EBIndexStyleCode::EB_INDEX_STYLE_DELETE;
    label[0] = '\0';
}
