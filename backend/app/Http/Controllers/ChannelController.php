<?php

namespace App\Http\Controllers;

use App\Models\Channel;
use App\Models\Stream;
use App\Models\User;
use Illuminate\Http\Request;
use Illuminate\Support\Facades\Auth;
use Illuminate\Support\Facades\Storage;
use Illuminate\Support\Str;
use function Laravel\Prompts\error;

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

    public function show($userId)
    {
        $user = User::findOrFail($userId);
        $channel = $user->channel;

        if (!$channel) {
            return response()->json(["error" => "Channel not found."], 404);
        }

        return response()->json([
            "channel" => $channel,
            "user" => $user
        ]);
    }

    public function update(Request $request)
    {
        $user = Auth::user();
        $channel = $user->channel;

        $data = $request->validate([
            'description' => 'nullable|string',
        ]);

        $channel->update($data);

        return response()->json(["channel" => $channel]);
    }
    public function start(Request $request)
    {
        $request->validate([
            'stream_title' => 'nullable|string|max:255',
            'stream_description' => 'nullable|string',
            'stream_category' => 'nullable|string|max:255',
            'stream_key' => 'required|string',
            'thumbnail' => 'nullable',
        ]);

        $channel = Channel::where('stream_key', $request->stream_key)->firstOrFail();
        if(!$channel) {
            return response()->json(["error" => "Channel not found."], 404);
        }

        if ($channel->is_live) {
            return response()->json(["error" => "Channel is already live."], 400);
        }
        $path = null;
        if ($request->hasFile('thumbnail')) {
            // Delete old thumbnail if exists
            if ($channel->thumbnail && Storage::disk('public')->exists($channel->thumbnail)) {
                Storage::disk('public')->delete($channel->thumbnail);
            }

            // Store new thumbnail
            $path = $request->file('thumbnail')->store('thumbnails', 'public');
        }

        $channel->update([
            'is_live' => true,
            'stream_title' => request('stream_title'),
            'stream_description' => request('stream_description'),
            'stream_category' => request('stream_category'),
            'thumbnail' => $path,
        ]);
        return response()->json(["message" => "Channel is live."], 200);

    }

    // End a stream (called when streaming ends)
    public function end(Request $request)
    {
        $request->validate([
            'user_id' => 'required|integer|exists:users,id',
        ]);
        $channel = Channel::where('user_id', $request->user_id)->firstOrFail();
        if(!$channel) {
            return response()->json(["error" => "Channel not found."], 404);
        }
        $channel->is_live = false;
        $channel->save();

        return response()->json(['message' => 'Stream ended']);
    }
    public function regenerateKey($id)
    {
        $channel = Channel::where('id', $id)->firstOrFail();
        if (!$channel) {
            return response()->json(["error" => "Channel not found."], 404);
        }
        $user = Auth::user();
        if( $user->id != $channel->user_id) {
            return response()->json(["error" => "Unauthorized user."], 403);
        }

        // Generate a new unique key
        do {
            $newKey = Str::random(32);
        } while (Channel::where('stream_key', $newKey)->exists());

        $channel->stream_key = $newKey;
        $channel->save();

        return response()->json(['channel' => $channel]);
    }
    public function settings($id)
    {
        $channel = Channel::where('id', $id)->firstOrFail();
        if (!$channel) {
            return response()->json(["error" => "Channel not found."], 404);
        }
        $user = Auth::user();
        if( $user->id != $channel->user_id) {
            return response()->json(["error" => "Unauthorized user."], 403);
        }
        $host = request()->getHost();
        return response()->json(["channel" => $channel, "host" => $host]);
    }
    public function stream($id)
    {
        $channel = Channel::where('id', $id)->firstOrFail();
        if (!$channel) {
            return response()->json(["error" => "Channel not found."], 404);
        }
        $host = request()->getHost();
        return response()->json(["channel" => $channel, "host" => $host]);
    }
    public function index() {
        $channels = Channel::where('is_live', true)->get();

        foreach ($channels as $channel) {

                $channel->thumbnail = $channel->thumbnail
                    ? asset('storage/' . $channel->thumbnail)
                    : null;

        }

        return response()->json(["channels" => $channels]);
    }
}
