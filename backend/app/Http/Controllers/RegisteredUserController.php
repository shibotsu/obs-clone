<?php

namespace App\Http\Controllers;

use App\Models\User;
use Illuminate\Http\Request;
use Illuminate\Support\Facades\Auth;
use Illuminate\Support\Facades\Hash;
use Tymon\JWTAuth\Facades\JWTAuth;


class RegisteredUserController extends Controller
{

    public function store() {
        $validatedAttributes = request()->validate([
            'username' => ['required', 'string', 'max:255', 'unique:users,username'],
            'email' => ['required', 'string', 'email', 'max:255', 'unique:users,email'],
            'password' => ['required', 'string', 'min:8'],
            'birthday' => ['required', 'date', 'before:today'],
        ]);

        $user = User::create($validatedAttributes);
        $token = JWTAuth::fromUser($user);
        return response()->json(['token' => $token], 201);
    }

    public function editUsername() {
        $validatedAttributes = request()->validate([
            'username' => ['required', 'string', 'max:255'],
            'new_username' => ['required', 'string', 'max:255', 'unique:users,username'],
        ]);
        $user = Auth::user();
        if ($validatedAttributes['username'] != $user->username) {
            return response()->json(['Wrong username'], 400);
        }
        $user->username = $validatedAttributes['new_username'];
        $user->save();
        return response()->json(['user' => $user, 'message' => 'Username successfully updated'], 200);
    }
    public function editEmail() {
        $validatedAttributes = request()->validate([
            'email' => ['required', 'string', 'email', 'max:255'],
            'new_email' => ['required', 'string', 'email', 'max:255', 'unique:users,email'],
        ]);
        $user = Auth::user();
        if ($validatedAttributes['email'] != $user->email) {
            return response()->json(['Wrong email'], 400);
        }
        $user->email = $validatedAttributes['new_email'];
        $user->save();
        return response()->json(['user' => $user, 'message' => 'Email successfully updated'], 200);
    }

    public function editPassword()
    {
        $validatedAttributes = request()->validate([
            'password' => ['required', 'string', 'min:8'],
            'new_password' => ['required', 'string', 'min:8'],
        ]);

        $user = Auth::user();

        if (!Hash::check($validatedAttributes['password'], $user->password)) {
            return response()->json(['message' => 'Wrong password'], 400);
        }

        $user->password = Hash::make($validatedAttributes['new_password']);
        $user->save();

        return response()->json(['user' => $user, 'message' => 'Password successfully updated'], 200);
    }


    public function destroy() {
        $user = Auth::user();
        if (!$user || !User::find($user->id)) {
            return response()->json(['error' => 'Unauthorized'], 401);
        }
        JWTAuth::invalidate(JWTAuth::getToken());
        $user->delete();
        return response()->json(['message' => 'User successfully deleted'], 200);
    }

}
