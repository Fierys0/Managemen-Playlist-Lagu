#pragma once
#include "fumbo.hpp"
#include "playlistData.hpp"
#include <string>

// Utilitas ekstraksi metadata VLC.
// Menggunakan libvlc (VLC 3.x C API langsung) untuk mengurai metadata file
// audio lokal. VLC hanya digunakan untuk metadata, pemutaran menggunakan Fumbo
// AudioManager.
namespace VlcMeta {

// Inisialisasi instance libvlc (panggil sekali saat startup).
void Init();

// Bebaskan instance libvlc (panggil saat shutdown).
void Shutdown();

// Urai file audio lokal dan kembalikan Track yang sudah terisi.
// Jika gagal, judul akan menggunakan nama file sebagai cadangan.
Track GetTrackInfo(const std::string &filePath);

// Muat tekstur sampul lagu dari path yang tersimpan di track.coverArtPath.
// Menggunakan assets/images/placeholder.png jika path kosong atau file tidak
// ada.
Texture2D LoadCoverTexture(const std::string &coverArtPath);

} // namespace VlcMeta
