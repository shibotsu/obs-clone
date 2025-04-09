<?php

namespace App\Http\Controllers;

use App\Models\User;
use Illuminate\Http\Request;
use Illuminate\Support\Facades\Storage;

class ProfileController extends Controller
{
    public function show(Request $request)
    {
        $user = $request->user();

        if (!$user) {
            return response()->json(['error' => 'Unauthorized'], 401);
        }

        $userData = $user->toArray();
        $userData['profile_picture'] = $user->profile_picture
            ? asset('storage/' . $user->profile_picture)
            : null;

        return response()->json($userData);
    }

    public function changePicture(Request $request)
    {
        $request->validate([
            'profile_picture' => 'required|image|mimes:jpeg,png,jpg,gif|max:2048'
        ]);

        $user = auth()->user();
        if (!$user) {
            return response()->json(['error' => 'Unauthorized'], 401);
        }
        if ($user->profile_picture && Storage::disk('public')->exists($user->profile_picture)) {
            Storage::disk('public')->delete($user->profile_picture);
        }
        $path = $request->file('profile_picture')->store('profile_pictures', 'public');
        $user->profile_picture = $path;
        $user->save();

        return response()->json([
            'message' => 'Profile picture updated successfully!',
            'url' => asset('storage/' . $path)
        ]);
    }
    public function channel($id) {
        $channel = User::find($id);
        if(!$channel) {
            return response()->json(['error' => 'Channel does not exist'], 401);
        }
        $channelData = $channel->only(['username', 'number_of_followers', 'profile_picture']);
        $channelData['profile_picture'] = $channel->profile_picture
            ? asset('storage/' . $channel->profile_picture)
            : null;
        return response()->json(['channel' => $channelData]);
    }
    public function follow($id)
    {
        $user = User::find($id);
        if (!$user) {
            return response()->json(['message' => 'User not found.'], 404);
        }

        if (auth()->id() === $user->id) {
            return response()->json(['message' => 'You cannot follow yourself.'], 400);
        }

        if (auth()->user()->following()->where('following_id', $user->id)->exists()) {
            return response()->json(['message' => 'You are already following this user.'], 400);
        }

        auth()->user()->following()->attach($user->id);
        $user->number_of_followers++;
        $user->save();

        return response()->json(['message' => 'Followed successfully']);
    }


    public function unfollow($id)
    {
        $user = User::find($id);
        if (!$user) {
            return response()->json(['message' => 'User not found.'], 404);
        }

        if (auth()->id() === $user->id) {
            return response()->json(['message' => 'You cannot unfollow yourself.'], 400);
        }
        auth()->user()->following()->detach($user->id);
        $user->number_of_followers--;
        return response()->json(['message' => 'Unfollowed successfully']);
    }

    public function followers($id)
    {
        $user = User::find($id);
        if (!$user) {
            return response()->json(['message' => 'User not found.'], 404);
        }
        $followers = $user->followers;
        return response()->json(['followers' => $followers]);
    }

    public function following()
    {
        $following = auth()->user()->following;
        return response()->json(["following" => $following]);
    }
    public function mostFollowed() {
        $users = User::orderByDesc('number_of_followers')->take(8)->get(['id', 'username', 'number_of_followers', 'profile_picture']);
        foreach ($users as $user) {
            $user->profile_picture = $user->profile_picture
                ? asset('storage/' . $user->profile_picture)
                : null;
        }

        return response()->json(["users" => $users]);
    }
}
