<?php

namespace App\Http\Controllers;

use App\Models\User;
use Illuminate\Http\Request;

class ChannelController extends Controller
{
    public function search(Request $request) {
        $query = $request->input('query');
        $users = User::where('username', 'like', "%{$query}%")->get();
        foreach ($users as $user) {
            $user->profile_picture = $user->profile_picture
                ? asset('storage/' . $user->profile_picture)
                : null;
        }
        return response()->json(["users" => $users]);

    }
}
