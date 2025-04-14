<?php

namespace App\Http\Controllers;
use Illuminate\Http\Request;
use App\Models\Message;
use App\Events\MessageSent;


// app/Http/Controllers/MessageController.php

namespace App\Http\Controllers;

use App\Models\Message;
use App\Events\MessageSent;
use Illuminate\Http\Request;

class MessageController extends Controller
{
    // Definiraj metodu store koja sprema poruku
    public function store(Request $request)
    {
        // Validacija podataka
        $request->validate([
            'receiver_id' => 'required|exists:users,id', // Provjeri postoji li korisnik s tim ID-jem
            'message' => 'required|string',
        ]);

        // Kreiranje nove poruke u bazi podataka
        $message = Message::create([
            'sender_id' => auth()->id(),  // Trenutno prijavljeni korisnik
            'receiver_id' => $request->receiver_id, // ID primatelja
            'message' => $request->message, // Sam tekst poruke
        ]);

        // Emitiranje događaja (MessageSent) na kanal za primatelja
        broadcast(new MessageSent($message))->toOthers();

        // Vraćanje odgovora s podacima o poruci
        return response()->json($message, 201);
    }
}


